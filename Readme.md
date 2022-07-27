# Tagger

A script that search files based on tags enclosed inside the filename.

I use it a lot so maybe it will be useful to someone.

Simple and stupid, not safe at all (perform Bash eval and recursive
calls), but it works.

## Dependencies

- GNU grep
- GNU find
- fzf

## Installation

(Assuming ~/.local/bin is in your path):

```bash
cd tagger
cp tagger.sh ~/.local/bin/
chmod +x ~/.local/bin/tagger.sh
```

## Usage

The files should be nammed with the tags at the beginning of the file,
enclosed in parentheses:

```bash
(tag1,tag2) foo.txt
```

```bash
OPTIONS:
   -f Tag1 Tag2 TagN Return matching files (name only)
   -p Tag1 Tag2 TagN Return matching files with the path
   -x Tag1 Tag2 TagN Same as -p but pass the result to fzf
   -l                List all existing tags
   -a                List all tagged files
   -n                List all files not tagged
```

## Rationale

    - Why not use an index file?

        Because I wanted something really simple, and the indexing
        part add a level of complexity (ex: what if you rename or move
        a file?).

    - Why use this format?

        It may not be the prettier, but it remove ambiguity and ease
        the parsing. I am yet to see a regular file whose name begin
        with a paren.

    - I still don't like this format, are there any alternatives?

        Yes, have a look at these two projects, they respectively use
        this `file -- tag1 tag2.txt` and this `file[tag1,tag2].txt`
        format.

        https://github.com/novoid/filetags

        https://github.com/mdom/squaretag

## Demo
