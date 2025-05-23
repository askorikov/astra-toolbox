#!/bin/sh

case `uname` in
  Darwin*)
    CC="gcc -stdlib=libstdc++"
    ;;
esac

cd $SRC_DIR/python/
CPPFLAGS="-DASTRA_CUDA -DASTRA_PYTHON $CPPFLAGS -I$SRC_DIR/ -I$SRC_DIR/include -I$SRC_DIR/lib/include" CC=$CC CFLAGS="-std=c++17" python ./builder.py build install
