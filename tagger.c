#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#define MAX_PATH_LENGTH 2048
#define MAX_FILENAME_LENGTH 256
#define MAX_TAGS_LENGTH 2048
#define INITIAL_TAG_CAPACITY 32

const char *BASEREGEX = "+.*+@.*";
const int MAXTAGNB = 30;

struct FileTuple {
    char path[MAX_PATH_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    char tags[MAX_TAGS_LENGTH];
};

struct TagSet {
    char **tags;
    size_t count;
    size_t capacity;
};

// Function prototypes for TagSet operations.
struct TagSet* create_tag_set(size_t initial_capacity);
void destroy_tag_set(struct TagSet *set);
void add_tag(struct TagSet *set, const char *tag);
void print_tag_set(const struct TagSet *set);

// Function prototypes for main operations.
void search_files(const char *path, regex_t *regex, struct FileTuple **matches, int *match_count);
void extract_tags(const char *filename, char *tags);
void find_by_tags(int num_tags, char tags[][MAXTAGNB], int match_count, struct FileTuple *matches, int logical_and);
void list_tags(regex_t regex, struct FileTuple *matches, const char *BASEDIR);

char* create_new_filename(const char* original_filename, const char* tag_to_remove);
int rename_file_remove_tag(const char* old_path, const char* new_path);
void remove_tag_from_files(const char* tag_to_remove, regex_t *regex, struct FileTuple *matches, const char* BASEDIR);

struct TagSet*
create_tag_set(size_t initial_capacity)
{
    struct TagSet *set = malloc(sizeof(struct TagSet));
    if (!set) {
        fprintf(stderr, "Failed to allocate TagSet\n");
        exit(EXIT_FAILURE);
    }

    set->tags = malloc(sizeof(char*) * initial_capacity);
    if (!set->tags) {
        free(set);
        fprintf(stderr, "Failed to allocate tags array\n");
        exit(EXIT_FAILURE);
    }

    set->count = 0;
    set->capacity = initial_capacity;
    return set;
}

void
destroy_tag_set(struct TagSet *set)
{
    if (!set) return;

    for (size_t i = 0; i < set->count; i++) {
        free(set->tags[i]);
    }
    free(set->tags);
    free(set);
}

void
add_tag(struct TagSet *set, const char *tag)
{
    // Check for duplicates.
    for (size_t i = 0; i < set->count; i++) {
        if (strcmp(set->tags[i], tag) == 0) {
            return;  // Tag already exists
        }
    }

    // Resize if needed.
    if (set->count >= set->capacity) {
        size_t new_capacity = set->capacity * 2;
        char **new_tags = realloc(set->tags, sizeof(char*) * new_capacity);
        if (!new_tags) {
            fprintf(stderr, "Failed to resize tags array\n");
            return;
        }

        set->tags = new_tags;
        set->capacity = new_capacity;
    }

    // Add new tag.
    set->tags[set->count] = strdup(tag);
    if (!set->tags[set->count]) {
        fprintf(stderr, "Failed to duplicate tag string\n");
        return;
    }
    set->count++;
}

void
print_tag_set(const struct TagSet *set)
{
    for (size_t i = 0; i < set->count; i++) {
        printf("%s\n", set->tags[i]);
    }
}

void
list_tags(regex_t regex, struct FileTuple *matches, const char *BASEDIR)
{
    int match_count = 0;
    // Fix the pointer type issue.
    struct FileTuple **matches_ptr = &matches;
    search_files(BASEDIR, &regex, matches_ptr, &match_count);

    struct TagSet *tag_set = create_tag_set(32);  // Start with space for 32 tags.

    for (int i = 0; i < match_count; i++) {
        char tags_copy[2048];
        strncpy(tags_copy, matches[i].tags, sizeof(tags_copy) - 1);
        tags_copy[sizeof(tags_copy) - 1] = '\0';

        char *token = strtok(tags_copy, ",");
        while (token != NULL) {
            // Remove leading/trailing whitespace.
            while (*token == ' ') token++;
            char *end = token + strlen(token) - 1;
            while (end > token && *end == ' ') {
                *end = '\0';
                end--;
            }

            if (*token) {  // Only add non-empty tags.
                add_tag(tag_set, token);
            }
            token = strtok(NULL, ",");
        }
    }

    print_tag_set(tag_set);
    destroy_tag_set(tag_set);
}

void
extract_tags(const char *filename, char *tags)
{
    const char *start = strchr(filename, '+');

    if(start) {
        const char *end = strchr(start + 1, '+');

        if(end) {
            size_t tag_len = (size_t)(end - start - 1);
            strncpy(tags, start + 1, tag_len);
            tags[tag_len] = '\0'; // Null-termination.
        }
        else {
            fprintf(stderr, "Error (extract_tags): 'end' is a NULL pointer.\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr, "Error (extract_tags): 'start' is a NULL pointer.\n");
        exit(EXIT_FAILURE);
    }
}

void
search_files(const char *path, regex_t *regex, struct FileTuple **matches, int *match_count)
{
    struct dirent *entry;
    DIR *dp = opendir(path);

    if(dp == NULL) {
        perror("opendir");
        return;
    }

    while((entry = readdir(dp)) != NULL) {
        if(entry->d_type == DT_DIR) {
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char new_path[MAX_PATH_LENGTH];
                int result = snprintf(new_path, MAX_PATH_LENGTH, "%s/%s", path, entry->d_name);
                if (result >= MAX_PATH_LENGTH || result < 0) {
                    fprintf(stderr, "Path too long or encoding error\n");
                    continue;
                }
                search_files(new_path, regex, matches, match_count); // Recursive call for subdirs.
            }
        }
        else if(entry->d_type == DT_REG) {
            if(regexec(regex, entry->d_name, 0, NULL, 0) == 0) {
                struct FileTuple match;
                // subtract 1 from the destination size to ensure that there is room for the null-terminator in the destination buffer.
                strncpy(match.path, path, sizeof(match.path) - 1);
                match.path[sizeof(match.path) - 1] = '\0'; // Null-termination.
                strncpy(match.filename, entry->d_name, sizeof(match.filename));
                extract_tags(entry->d_name, match.tags);
                (*matches)[*match_count] = match;
                (*match_count)++;
            }
        }
    }
    closedir(dp);
}

void
find_by_tags(int num_tags, char tags[][MAXTAGNB], int match_count, struct FileTuple *matches, int logical_and)
{
    for(int i = 0; i < match_count; i++) {
        // Set to 1 for logical AND, 0 for logical OR.
        int contains_all_tags = logical_and;

        for(int j = 0; j < num_tags; j++) {
            if(strstr(matches[i].tags, tags[j]) == NULL) {
                // If a tag is not found in the file's tags, set the flag to 0 for logical OR.
                if(logical_and) {
                    contains_all_tags = 0;
                    break; // No need to check further for logical AND.
                }
            }
            else {
                // If a tag is found in the file's tags, set the flag to 1 for logical OR.
                if(!logical_and) {
                    contains_all_tags = 1;
                    break; // No need to check further for logical OR.
                }
            }
        }

        if(contains_all_tags) {
            printf("%s/%s\n", matches[i].path, matches[i].filename);
        }
    }
}

char*
create_new_filename(const char* original_filename, const char* tag_to_remove)
{
    char* new_filename = malloc(MAX_FILENAME_LENGTH);
    if (!new_filename) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // First, find the tags section.
    const char* start = strchr(original_filename, '+');
    const char* end = strrchr(original_filename, '+');

    if (!start || !end || start == end) {
        free(new_filename);
        return NULL;
    }

    // Extract the tags.
    size_t tags_len = (size_t)(end - start - 1);
    char* tags = malloc(tags_len + 1);
    if (!tags) {
        free(new_filename);
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strncpy(tags, start + 1, tags_len);
    tags[tags_len] = '\0';

    // Split tags and rebuild without the removed tag.
    char* new_tags = malloc(tags_len + 1);
    if (!new_tags) {
        free(tags);
        free(new_filename);
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_tags[0] = '\0';

    char* tag = strtok(tags, ",");
    int found_tag = 0;
    int is_first = 1;

    while (tag != NULL) {
        // Remove leading/trailing spaces.
        while (*tag == ' ') tag++;
        char* tag_end = tag + strlen(tag) - 1;
        while (tag_end > tag && *tag_end == ' ') {
            *tag_end = '\0';
            tag_end--;
        }

        if (strcmp(tag, tag_to_remove) != 0) {
            if (!is_first) {
                strcat(new_tags, ",");
            }
            strcat(new_tags, tag);
            is_first = 0;
        }
        else {
            found_tag = 1;
        }
        tag = strtok(NULL, ",");
    }

    if (!found_tag) {
        free(tags);
        free(new_tags);
        free(new_filename);
        return NULL;
    }

    // Reconstruct the filename.
    if (strlen(new_tags) > 0) {
        // Still has other tags.
        snprintf(new_filename, MAX_FILENAME_LENGTH, "%.*s+%s+%s",
                 (int)(start - original_filename), original_filename,
                 new_tags,
                 end + 1);
    }
    else {
        // No tags left, remove the tag wrapper and '@' completely.
        const char* filename_start = strchr(end + 1, '@');
        if (filename_start) {
            filename_start++; // Skip the '@'
            snprintf(new_filename, MAX_FILENAME_LENGTH, "%.*s%s",
                     (int)(start - original_filename), original_filename,
                     filename_start);
        }
        else {
            // Fallback if '@' is not found (shouldn't happen with valid filenames).
            snprintf(new_filename, MAX_FILENAME_LENGTH, "%.*s%s",
                     (int)(start - original_filename), original_filename,
                     end + 1);
        }
    }

    free(tags);
    free(new_tags);
    return new_filename;
}

int
rename_file_remove_tag(const char* old_path, const char* new_path)
{
    if (strcmp(old_path, new_path) == 0) {
        return 0;
    }

    // Check if destination already exists.
    if (access(new_path, F_OK) == 0) {
        fprintf(stderr, "Destination file already exists: %s\n", new_path);
        return -1;
    }

    if (rename(old_path, new_path) != 0) {
        perror("Failed to rename file");
        return -1;
    }

    return 0;
}

int
join_path_safely(char *dest, size_t dest_size, const char *dir, const char *file)
{
    int result = snprintf(dest, dest_size, "%s/%s", dir, file);

    // Check if truncation occurred.
    if (result < 0 || (size_t)result >= dest_size) {
        fprintf(stderr, "Path too long: %s/%s\n", dir, file);
        return -1;
    }

    return 0;
}

void
remove_tag_from_files(const char* tag_to_remove, regex_t *regex, struct FileTuple *matches, const char* BASEDIR)
{
    int match_count = 0;
    struct FileTuple **matches_ptr = &matches;
    search_files(BASEDIR, regex, matches_ptr, &match_count);

    int files_modified = 0;
    int errors = 0;

    for (int i = 0; i < match_count; i++) {
        char* new_filename = create_new_filename(matches[i].filename, tag_to_remove);

        if (new_filename) {
            char old_path[MAX_PATH_LENGTH];
            char new_path[MAX_PATH_LENGTH];

            // Use safe path joining.
            if (join_path_safely(old_path, sizeof(old_path),
                                 matches[i].path, matches[i].filename) != 0) {
                free(new_filename);
                errors++;
                continue;
            }

            if (join_path_safely(new_path, sizeof(new_path),
                                 matches[i].path, new_filename) != 0) {
                free(new_filename);
                errors++;
                continue;
            }

            if (rename_file_remove_tag(old_path, new_path) == 0) {
                printf("Renamed: %s -> %s\n", matches[i].filename, new_filename);
                files_modified++;
            } else {
                errors++;
            }

            free(new_filename);
        }
    }

    printf("\nSummary:\n");
    printf("Files processed: %d\n", match_count);
    printf("Files modified: %d\n", files_modified);
    if (errors > 0) {
        printf("Errors encountered: %d\n", errors);
    }
}

int
main(int argc, char *argv[])
{
    regex_t regex;
    if(regcomp(&regex, BASEREGEX, 0) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        exit(1);
    }

    // Either use an environment variable, or set the value here (do not forget to comment the second line):
    // const char *BASEDIR = "/your/path/here";
    const char *BASEDIR = getenv("TAGGER_PATH");

    if(BASEDIR == NULL) {
        fprintf(stderr, "Please set the TAGGER_PATH environment variable, or use an hardcoded value like explained above.");
        exit(EXIT_FAILURE);
    }

    long unsigned int max_matches = 10000;
    struct FileTuple *matches = malloc(sizeof(struct FileTuple) * max_matches);

    // TODO: use an enum.
    // Default is logical AND.
    int logical_and = 1;

    // TODO: move all this part to a function.
    int opt;

    struct option long_options[] = {
        {"or", no_argument, 0, 'o'},
        {"list", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {"remove", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "olhr:", long_options, NULL)) != -1) {
        switch (opt) {
        case 'o':
            logical_and = 0;
            break;
        case 'l':
            list_tags(regex, matches, BASEDIR);
            return 0;
        case 'h':
            fprintf(stderr, "Usage: %s [OPTIONS] [tag1 tag2 ... tagN]\n", argv[0]);
            fprintf(stderr, "  -o, --or          Use logical OR\n");
            fprintf(stderr, "  -l, --list        List tags\n");
            fprintf(stderr, "  -r, --remove=TAG  Remove specified tag from files\n");
            fprintf(stderr, "  -h, --help        Show this help message\n");
            return 0;
        case 'r':
            remove_tag_from_files(optarg, &regex, matches, BASEDIR);
            return 0;
        default:
            fprintf(stderr, "Invalid option. Use '%s --help' for usage information.\n", argv[0]);
            exit(1);
        }
    }

    // Process tags from CLI.
    char tags_to_find[argc - optind][MAXTAGNB];
    int num_tags_to_find = 0;
    for(int i = optind; i < argc; i++) {
        if(num_tags_to_find < MAXTAGNB) {
            strncpy(tags_to_find[num_tags_to_find], argv[i], sizeof(tags_to_find[num_tags_to_find]));
            num_tags_to_find++;
        }
        else {
            fprintf(stderr, "Too many tags specified.\n");
            exit(1);
        }
    }

    int match_count = 0;

    search_files(BASEDIR, &regex, &matches, &match_count);

    find_by_tags(num_tags_to_find, tags_to_find, match_count, matches, logical_and);

    regfree(&regex);
    free(matches);

    return 0;
}
