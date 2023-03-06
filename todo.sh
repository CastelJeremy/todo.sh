#!/bin/bash

if [ -z $TODOSH_DATA_FILE ]; then
    TODOSH_DATA_FILE=/tmp/todo.txt
fi

STRIKE="\e[9m"
RESET="\e[0m"
COLOR="\e[49;38;5;81m"

parse_task() {
    done=`echo $2 | cut -c 1`
    str=`echo $2 | cut -c 2-`
    if [ "$done" = "1" ]; then
        str="${STRIKE}${str}${RESET}"
    fi

    if [ $1 -lt 10 ]; then
        echo -e "${COLOR}0${i}${RESET} $str"
    else
        echo -e "${COLOR}$i${RESET} $str"
    fi
}

list_tasks() {
    i=0;
    cat $TODOSH_DATA_FILE | while read line; do
        i=$((i+1))
        parse_task $i "$line"
    done
}

show_error() {
    echo "todo.sh: $1"
    echo "Try 'todo.sh --help' for more information."
}

if [ ! -e $TODOSH_DATA_FILE ]; then
    echo "todo.sh: cannot stat '$TODOSH_DATA_FILE': No such file"
    exit
fi

if [ ! -w $TODOSH_DATA_FILE ]; then
    echo "todo.sh: cannot stat '$TODOSH_DATA_FILE': Permission denied"
    exit
fi

SIZE=`<$TODOSH_DATA_FILE wc -l`

if [ "$1" = "a" ] || [ "$1" = "-a" ] || [ "$1" = "--add" ]; then
    if [ $# -lt 2 ]; then
        show_error "missing content operand after '$1'"
        exit
    fi
    echo "0$2" >> $TODOSH_DATA_FILE
elif [ "$1" = "r" ] || [ "$1" = "-rm" ] || [ "$1" = "--remove" ]; then
    if [ $# -lt 2 ]; then
        show_error "missing number operand after '$1'"
        exit
    elif [ $2 -lt 1 ] || [ $2 -gt $SIZE ]; then
        show_error "invalid number operand '$2'"
        exit
    fi

    {
        printf '%dd\n' "$2"
        printf '%s\n' w q
    } | ed -s $TODOSH_DATA_FILE
elif [ "$1" = "u" ] || [ "$1" = "-u" ] || [ "$1" = "--update" ]; then
    if [ $# -lt 2 ]; then
        show_error "missing number operand after '$1'"
        exit
    elif [ $2 -lt 1 ] || [ $2 -gt $SIZE ]; then
        show_error "invalid number operand '$2'"
        exit
    fi

    line=`sed -n $2p $TODOSH_DATA_FILE`
    done=`echo $line | cut -c 1`
    clean_line=`echo $line | cut -c 2-`

    if [ "$done" = "0" ]; then
        {
            printf '%dc\n' "$2"
            printf '1%s\n.\n' "$clean_line"
            printf '%s\n' w q
        } | ed -s $TODOSH_DATA_FILE
    elif [ "$done" = "1" ]; then
        {
            printf '%dc\n' "$2"
            printf '0%s\n.\n' "$clean_line"
            printf '%s\n' w q
        } | ed -s $TODOSH_DATA_FILE
    fi
elif [ "$1" = "s" ] || [ "$1" = "-s" ] || [ "$1" = "--switch" ]; then
    if [ $# -lt 3 ]; then
        show_error "missing number operand after '${@: -1}'"
        exit
    elif [ $2 -lt 1 ] || [ $2 -gt $SIZE ]; then
        show_error "invalid number operand '$2'"
        exit
    elif [ $3 -lt 1 ] || [ $3 -gt $SIZE ]; then
        show_error "invalid number operand '$3'"
        exit
    fi

    {
        printf '%dm%d\n' "$2" "$3"
        printf '%d-m%d-\n' "$3" "$2"
        printf '%s\n' w q
    } | ed -s $TODOSH_DATA_FILE
elif [ "$1" = "h" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "Help section todo"
    exit
fi

list_tasks

