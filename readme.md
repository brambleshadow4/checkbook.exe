Checkbook.exe
=============

## A stupid tool for keeping track of financial transactions. ##

### About ###
This is a command line tool that I use to keep track of various bank accounts
that I have. I used to use an excel spread sheet for such purposes, but now that
I have multiple accounts, I wanted something I could just type transactions into
and let it handle everything else.

Currently, this uses native windows libraries and will not run on any other
operating systems.

### Usage ###

All financial data is stored in a file named "finances.db".

To create a new bank account, type `new accountName` into the console. You can
now see the current status of this account with `sum` (summary) or `s` for
short.

To create a new transaction, type `accountName +amount month/day "description"`
e.g.  
`debit +5 1/4 "adding 5 dollars"`  
`account2 -14.99 7/4/2018 "Squid hat"`  

If you make a mistake, you can edit it after the fact using the `edit` command

To view the last 20 transactions, type `last 20`. You can filter these to only
transactions for a specific account with `last 20 accountName`

A neat short hand for transferring money between two accounts exists. Its syntax
is `acc1 -> acc2 ammount month/day "message"`

Finally, to quit the application, type `quit` or `q` for short.

### Compilation ###

The project is compiled via the terminal using gcc and a one line bat file
included in this project, "make.bat"

	gcc -c sqlite3.c
	gcc -c printcolor.c
	make

The first two command only need to be run once, unless changes to sqlite3.c
 or printcolor.c are made.
