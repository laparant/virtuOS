# VirtuOS
## Team G3-E4
  * Paul Gaulier ~pgaulier
  * Alexandre Hervé ~aherve004
  * Louise Mouret ~lmouret001
  * Laurie-Anne Parant ~lparant

## Supervisor
  * Quentin Rouxel ~qrouxel

## Compilation
Run these commands into the build directory:
```shell
$> cmake -DUSE_PTHREAD=OFF <PATH TO THE ROOT PROJECT DIRECTORY>
$> make
```
This will build the library **libthread.so** and all the tests in your build directory.

If you want to compile the project with the standard library for thread, you should run these commands :
```shell
$> cmake -DUSE_PTHREAD=ON <PATH TO THE ROOT PROJECT DIRECTORY>
$> make
```

### Shared library
**With root privileges**

If you want to create programs with the **libthread.so** implemented by our team, you can execute `$> sudo make install` if you have the root rights.

Now, you only have to add `#include <thread.h>` into your program.

**Without root privileges**

The files **libthread.so** and **thread.h** are available in the directory **src/** in the root project directory. You can copy them and then use `gcc program.c -I<PATH_TO_HEADER> -L<PATH_TO_LIB>`.

## Tests
### Functionnality tests and memory checker
All the tests are in the directory **`tst`**. The commands below automate the execution with and without memory checker of the tests :

| Command                    | Description                                                   |
| -------------------------- |:--------------------------------------------------------------|
| `$> make test`             | run all the tests                                             |
| `$> make valgrind`         | run the tests with the memory checker valgrind (if installed).|

>**NB** : The tests *tst72* and *tst81* are not available with memory checker because they check the timeslice with 5% accuracy and a valgrind execution modifies too much the elapsed time. If you want to run the tests with valgrind, you should disable the *assert* and run it by command-line.

### Performance tests
The scripts in the directory **`perf`** generate time execution information.

| Command                                     | Description                                               | Enabled tests |
|---------------------------------------------|-----------------------------------------------------------|---------------|
|`./average.sh <PATH_TO_TEST>`                | Calculate the average time execution with 1000 executions | 01, 02, 11, 12 |
|`./nthread.sh <PATH_TO_TEST> <PATH_TO_FILE>` | Generate a file which contains the time execution by thread number with tread number from 1 to 1000.                                                                                                   | 21, 22, 23, 61 |
|`./nyield.sh <PATH_TO_TEST> <PATH_TO_FILE>`  | Generate a file which contains the time execution by yield number (from 1 to 10000) for 100 threads.                                                                                                                     | 31, 32 | 
|`./fibo.sh <PATH_TO_TEST> <PATH_TO_FILE>`    | Generate a file which contains the time execution by number (from 1 to 23). | 51 | 
> If you use **.dat** format file, you could use the file to draw graphs with *gnuplot*.

> If you use a script with another test (not in "Enabled tests" in the table above) the behavior is undefined.

> /!\ the file given by <PATH_TO_FILE> will be overwritten

No performance tests are availables for the tests 71, 72, 81 and 91 because they test the timeslice and stack overflow. The performance tests make sense only with usual behavior and these tests are about specific behavior in limit cases.

To compare our library with the standard library you should compile the project with `-DUSE_PTHREAD=OFF`, generate the files and then compile again the projects with `-DUSE_PTHREAD=ON` and generate others files. Then you should use gnuplot to draw graphs.

Example of a gnuplot generation :
```
$> (cd ../build; cmake -DUSE_PTHREAD=OFF ..; make)
$> ./fibo.sh ../build/tst/test_51_fibonacci fibo_thread.dat
$> (cd ../build; cmake -DUSE_PTHREAD=O ..; make)
$> ./fibo.sh ../build/tst/test_51_fibonacci fibo_pthread.dat
$> gnuplot
gnuplot> set title "Fibonacci comparison"
gnuplot> set xlabel "Number"
gnuplot> set ylabel "Time (seconds)"
gnuplot> plot "fibo_thread.dat" title "Fibonacci with our library" with linespoints, "fibo_pthread.dat" title "Fibonnaci with pthread" with linespoints
```

##Documentation
Doxygen has been used to generate automatic documentation. In the repertory `doc/` is a `Doxyfile.in` you can modify if you want to generate LaTex documentation. Only html documentation is enable yet. To generate documentation and see it (from the root of the project):
```
$> cd build/
$> make doc
$> firefox doc/html/index.html
```

