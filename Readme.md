# Tagger

    A script that search files based on tags enclosed inside the filename.

    I use it a lot so maybe it will be useful to someone.


There are three versions in this repo:

- *tagger.c*

    C version, simple, fast...

    This is the default version.

    Just set the _BASEDIR_ variable and run:
    ```
    make build
    make run
    ```

    The default regex use this file format:
    ```
    +tag1,tag2+@filename
    ```

- *tagger.hs*

    A version in Haskell I wrote mostly for fun.

    It still call grep behind the scene, but parse tags with Parsec.

    It's a bit slow (compared to C) and hard to optimize. I did find a
    way to make it kind of portable, but the compilation still require
    stack and undreds of MB, so I wasn't happy with it.

- *tagger.sh*

    Simple and stupid, not safe at all (perform Bash eval and recursive
    calls), but it works.
