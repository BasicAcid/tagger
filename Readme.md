# Tagger

A script that search files based on tags enclosed inside the filename.

I use it a lot so maybe it will be useful to someone.

Simple and stupid, not safe at all (perform Bash eval and recursive
calls), but it works.

## Usage

The files should be nammed with the tags at the beginning of the file,
enclosed in parentheses:

```
(tag1,tag2) foo
```

```
OPTIONS:
   -f Tag1 Tag2 TagN Return matching files
   -p Tag1 Tag2 TagN Return matching files with the path
   -l                List all existing tags
   -a                List all tagged files
   -n                List all files not tagged
```
