
if "%1" == "test" (
cmake -DMODE_TEST=ON -B build -S .
) else (
cmake -DMODE_TEST=OFF -B build -S .
)

cmake --build build --config Release