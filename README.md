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
$> cmake <PATH TO THE ROOT PROJECT DIRECTORY>
$> make
```
This will build the library **libthread.so** and all the tests in your build directory.

### Bibliothèque partagée
**With root privileges**

If you want to create programs with the **libthread.so** implemented by our team, you can execute `$> sudo make install` if you have the root rights.

Now, you only have to add `#include <thread.h>` into your program.

**Without root privileges**

The files **libthread.so** and **thread.h** are available in the directory **src/** in the root project directory. You can copy them and then use `gcc program.c -I<PATH_TO_HEADER> -L<PATH_TO_LIB>`.

## Tests and memory checker
All the tests are in the directory **`tst`**. The commands below automate the execution with and without memory checker of the tests :

| Command                    | Description                                                   |
| -------------------------- |:--------------------------------------------------------------|
| `$> make test`             | run all the tests                                             |
| `$> make valgrind`         | run the tests with the memory checker valgrind (if installed).|

>**NB** : The tests *tst72* and *tst81* are not available with memory checker because they check the timeslice with 5% accuracy and a valgrind execution modifies too much the elapsed time. If you want to run the tests with valgrind, you should disable the *assert* and run it by command-line.