## Censorship measurement tests

### Developing on Debian Wheezy:

To set up the development environment:

    $ sudo apt-get install liblua5.1-dev libldns-dev
    $ git clone https://github.com/gsathya/censorscope.git
    $ cd censorscope/
    $ make
    $ mkdir results
    $ ./censorscope

### Contributing
Please read the docs/HACKING file to understand the codebase.

If you'd like to contribute, these are some tasks that you might enjoy -
- Check out the issues for something to hack on
- Write moar experiments
- Grep for "TODO" in the source code and fix them
- Write tests for the experiments
- Write test specs for the experiments
- Write documentation on how to write an experiment
- Expand the HACKING file to include more information about the code
