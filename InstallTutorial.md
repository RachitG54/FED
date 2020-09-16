Installation Tutorial

Implementation Libraries - would recommend to install everything from binaries.
Install GMP, NTL. download version 6.1.2 for GMP from binaries and not the latest. Any GMP, NTL work. Check if they are linked properly.
Reference links:
https://www.shoup.net/ntl/doc/tour-unix.html

Steps:
See the link


Install boost, I installed 1.69.0 because I vaguely remember that was the one I used initially.
Reference links:
https://www.boost.org/users/history/
https://www.boost.org/doc/libs/1_69_0/more/getting_started/unix-variants.html

Steps:
Download source
Cd into directory
Run ./bootstrap.sh
Build using ./b2
Install using sudo ./b2 install

See reference link 2 for the statements


Then install msgpack. (Both c and c++ version), c is not needed but I don’t want to change code dependencies and debug right now.

Steps:
For c version:
git clone https://github.com/msgpack/msgpack-c.git
cd msgpack-c
git checkout c_master
cmake .
Make
sudo make install

References:
https://github.com/msgpack/msgpack-c/tree/c_master

For c++ version:

$ git clone https://github.com/msgpack/msgpack-c.git
$ cd msgpack-c
$ git checkout cpp_master
$ cmake -DMSGPACK_CXX[11]=ON .
$ sudo make install

References:
https://github.com/msgpack/msgpack-c/tree/cpp_master

Install open ssl. Use a prior version (anything <= 1.0.2u works).
Latest doesn’t compile with RSA data structures. (could be fixed, but would require attending to the code).
I am using 1.0.2u right now. This is a good reference to change all paths. (I needed to fix those manually as I broke things somewhere when doing all this.)

https://www.howtoforge.com/tutorial/how-to-install-openssl-from-source-on-linux/

I would follow these instructions to the letter and am confident you won’t have an issue.


Install OblivC.

Reference: https://github.com/samee/obliv-c

Follow this reference and you should be good.
