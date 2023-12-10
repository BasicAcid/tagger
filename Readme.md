# Tagger

    A script that search files based on tags enclosed inside the filename.

    I use it a lot so maybe it will be useful to someone.


There are three versions in this repo:

- *tagger.sh*

    Simple and stupid, not safe at all (perform Bash eval and recursive
    calls), but it works.

- *tagger.hs*

    A version in Haskell I wrote mostly for fun.

    It still call grep behind the scene, but parse tags with Parsec.

    It's a bit slow (compared to C) and hard to optimize. I did find a
    way to make it kind of portable, but the compilation still require
    stack and undreds of MB, so I wasn't happy with it.

- *tagger.c*

    C version, simple, fast...

    Just set the _BASEDIR_ variable and run:
    ```
    make build
    make run
    ```

    The default regex use this file format:
    ```
    +tag1,tag2+@filename
    ```

## tagger.sh Readme (old)

### Dependencies

- GNU grep
- GNU find
- fzf

### Installation

(Assuming ~/.local/bin is in your path):

```bash
cd tagger
cp tagger.sh ~/.local/bin/
chmod +x ~/.local/bin/tagger.sh
```

### Usage

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

### Rationale

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
