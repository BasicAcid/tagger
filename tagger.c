#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>

const char *BASEREGEX = "+.*+@.*";
const int MAXTAGNB = 30;

struct FileTuple
{
    char path[2048];
    char filename[256];
    char tags[2048]; // Max combined length of tags.
};

// Linked list to store tags.
struct TagNode
{
    char tag[40];
    struct TagNode *next;
};

void
print_llist(struct TagNode *head)
{
    struct TagNode *current = head;

    while(current != NULL)
    {
        printf("%s\n", current->tag);
        current = current->next;
    }
}

void
remove_duplicates_llist(struct TagNode *head)
{
    struct TagNode *current = head;

    while(current != NULL)
    {
        struct TagNode *runner = current;
        while(runner->next != NULL)
        {
            if(strcmp(current->tag, runner->next->tag) == 0)
            {
                struct TagNode *temp = runner->next;
                runner->next = runner->next->next;
                free(temp);
            }
            else
            {
                runner = runner->next;
            }
        }
        current = current->next;
    }
}

void
extract_tags(const char *filename, char *tags)
{
    const char *start = strchr(filename, '+');

    if(start)
    {
        const char *end = strchr(start + 1, '+');

        if(end)
        {
            size_t tag_len = (size_t)(end - start - 1);
            strncpy(tags, start + 1, tag_len);
            tags[tag_len] = '\0'; // Null-termination.
        }
        else
        {
            fprintf(stderr, "Error (extract_tags): 'end' is a NULL pointer.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "Error (extract_tags): 'start' is a NULL pointer.\n");
        exit(EXIT_FAILURE);
    }
}


void
search_files(const char *path, regex_t *regex, struct FileTuple **matches, int *match_count)
{
    struct dirent *entry;
    DIR *dp = opendir(path);

    if(dp == NULL)
    {
        perror("opendir");
        return;
    }

    while((entry = readdir(dp)) != NULL)
    {
        if(entry->d_type == DT_DIR)
        {
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                char new_path[2048];
                snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
                search_files(new_path, regex, matches, match_count); // Recursive call for subdirs.
            }
        }
        else if(entry->d_type == DT_REG)
        {
            if(regexec(regex, entry->d_name, 0, NULL, 0) == 0)
            {
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
    for(int i = 0; i < match_count; i++)
    {
        int contains_all_tags = logical_and; // Set to 1 for logical AND, 0 for logical OR.

        for(int j = 0; j < num_tags; j++)
        {
            if(strstr(matches[i].tags, tags[j]) == NULL)
            {
                // If a tag is not found in the file's tags, set the flag to 0 for logical OR.
                if(logical_and)
                {
                    contains_all_tags = 0;
                    break; // No need to check further for logical AND.
                }
            }
            else
            {
                // If a tag is found in the file's tags, set the flag to 1 for logical OR.
                if(!logical_and)
                {
                    contains_all_tags = 1;
                    break; // No need to check further for logical OR.
                }
            }
        }

        if(contains_all_tags)
        {
            printf("%s/%s\n", matches[i].path, matches[i].filename);
        }
    }
}


// Create a new node and initialize it with a tag.
struct TagNode
*create_tag_node(const char *tag)
{
    struct TagNode *new_node = (struct TagNode*)malloc(sizeof(struct TagNode));

    if(new_node == NULL)
    {
        exit(1);
    }

    strncpy(new_node->tag, tag, sizeof(new_node->tag));
    new_node->tag[sizeof(new_node->tag) - 1] = '\0'; // Ensure null-termination.

    new_node->next = NULL;

    return new_node;
}

void
list_tags(regex_t regex, struct FileTuple *matches, const char *BASEDIR)
{
    int match_count = 0;
    search_files(BASEDIR, &regex, &matches, &match_count);

    struct TagNode *tag_list = NULL;

    for(int i = 0; i < match_count; i++)
    {
        char *token = strtok(matches[i].tags, ",");
        while(token != NULL)
        {
            struct TagNode* newNode = create_tag_node(token);
            newNode->next = tag_list;
            tag_list = newNode;
            token = strtok(NULL, ",");
        }
    }

    remove_duplicates_llist(tag_list);

    print_llist(tag_list);
}



int
main(int argc, char *argv[])
{
    regex_t regex;
    if(regcomp(&regex, BASEREGEX, 0) != 0)
    {
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
    struct FileTuple *matches = (struct FileTuple *)malloc(sizeof(struct FileTuple) * max_matches);

    // Default is logical AND.
    int logical_and = 1;

    int opt;

    struct option long_options[] = {
        {"or", no_argument, 0, 'o'},
        {"list", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},  // Change 'required_argument' to 'no_argument'
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "olh", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'o':
            logical_and = 0; // Logical OR.
            break;
        case 'l':
            list_tags(regex, matches, BASEDIR);
            return 0;
            break;
        case 'h':
            fprintf(stderr, "Usage: %s [OPTIONS] [tag1 tag2 ... tagN]\n", argv[0]);
            fprintf(stderr, "  -o, --or          Use logical OR\n");
            fprintf(stderr, "  -l, --list        List tags\n");
            fprintf(stderr, "  -h, --help        Show this help message\n");
            return 0;
        default:
            fprintf(stderr, "Invalid option. Use '%s --help' for usage information.\n", argv[0]);
            exit(1);
        }
    }

    // Process tags from CLI.
    char tags_to_find[argc - optind][MAXTAGNB];
    int num_tags_to_find = 0;
    for(int i = optind; i < argc; i++)
    {
        if(num_tags_to_find < MAXTAGNB)
        {
            strncpy(tags_to_find[num_tags_to_find], argv[i], sizeof(tags_to_find[num_tags_to_find]));
            num_tags_to_find++;
        }
        else
        {
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
