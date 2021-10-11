cd toolchains
robocopy ./ %CONDA_PREFIX% h2o.cmake
robocopy ./ %CONDA_PREFIX%/Library/plugins applocal.ps1
robocopy ./ %CONDA_PREFIX%/Library/plugins qtdeploy.ps1
robocopy ./ %CONDA_PREFIX% applocal.ps1
cd ../

mkdir build
cd build

cmake -G "Visual Studio 15 2017" -A "x64" -T "host=x64" ^
    -DCMAKE_BUILD_TYPE=Release ^
    ../

cd ../