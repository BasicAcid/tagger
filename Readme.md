# Tagger

    A script that search files based on tags enclosed inside the filename.

    I use it a lot so maybe it will be useful to someone.


There are three versions in this repo:

- *tagger.c*

    C version, simple, fast...

    This is the default version.

    To use it you need to set the default search path, you can either:

    - use an environment variable:
    ```
    export TAGGER_PATH="/home/user/your/documents"
    ```

    - set the variable directly inside the program, to do so search
      for this line inside main:

      ```C
      const char *BASEDIR = "/your/path/here";
      ```

    Compilation and installation (default path is ~/.local/bin/):
    ```bash
    make build
    make run
    ```

    The default regex use this file format:
    ```bash
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
