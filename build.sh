gcc -g -Wall -Wextra -Wpedantic -lX11 keychord.c
if [ $? -eq 0 ]; then
    ./a.out
    echo "program exited with code $?"
else
    echo "build failed"
fi
