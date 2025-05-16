
if "%1" == "test" (
cmake -DMODE_TEST=ON -G "Unix Makefiles" -B build -S .
) else (
cmake -DMODE_TEST=OFF -G "Unix Makefiles" -B build -S .
)

cd build
make
