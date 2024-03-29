#!/bin/sh
usage()
{
cat << EOF
usage: $0 OPTION TAG

Find files by tag.

The filenames should obey the following format:
(tag1,tag2,tag3) file.txt

OPTIONS:
   -f Tag1 Tag2 TagN Return matching files (name only)
   -p Tag1 Tag2 TagN Return matching files with the path
   -x Tag1 Tag2 TagN Same as -p but pass the result to fzf
   -l                List all existing tags
   -a                List all tagged files
   -n                List all files not tagged

Examples:
  tagger.sh -f papers
  tagger.sh -x "papers CS"
EOF
}

list_tags () {
    find . -name '*' -type f -printf "%f\\n" | grep '^(..*)' | grep -o -P '(?<=\().*(?=\)[[:space:]])' | sed -e 's:,:\n:g' | sort | uniq
}

list_tagged_files () {
    find . -name '*' -type f -printf "%f\\n" | grep '^(..*)'
}

list_tagged_files_fullpath () {
    find . -name '*' -type f -print | grep '\\/(..*)'
}

list_not_tagged () {
    find . -name '*' -type f -not -path "*/.git/*" -printf "%f\\n" | grep -v '^(..*)'
}

list_not_tagged_fullpath () {
    find . -name '*' -type f -not -path "*/.git/*" -print | grep -v '\/(..*)' --exclude-dir='.git'
}

# find . -name '*' -type f -printf "%f\n" | grep '^(.*tag.*)'
find_by_tags () {
    for var in $@
    do
        tag_to_grep="| grep --color=auto '^(.*${var}.*)'"
        str_to_grep="${str_to_grep} ${tag_to_grep}"
    done

    cmd="find . -name '*' -type f -printf \"%f\\\n\""

    final_command="$cmd $str_to_grep"

    eval "$final_command"
}

find_by_tags_fullpath () {
    for var in $@
    do
        tag_to_grep="| grep --color=auto '\\/(.*${var}.*)'"
        str_to_grep="${str_to_grep} ${tag_to_grep}"
    done

    cmd="find . -name '*' -type f -print"

    final_command="$cmd $str_to_grep"

    eval "$final_command"
}

xdg_open_from_fzf () {
    for var in $@
    do
        tag_to_grep="| grep --color=auto '\\/(.*${var}.*)'"
        str_to_grep="${str_to_grep} ${tag_to_grep}"
    done

    cmd="find . -name '*' -type f -print"

    final_command="$cmd $str_to_grep"

    eval "$final_command | fzf | xargs -I {} echo '\"{}\"' | xargs xdg-open"
}

while getopts ":hf:p:x:aln" OPTION; do
      case $OPTION in
          h) usage exit 1 ;;
          f) find_by_tags "${OPTARG}";;
          p) find_by_tags_fullpath "${OPTARG}";;
          x) xdg_open_from_fzf "${OPTARG}";;
          l) list_tags;;
          a) list_tagged_files;;
          n) list_not_tagged_fullpath;;
          *) echo "Wrong flag"
      esac
done
