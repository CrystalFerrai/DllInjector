# DLL Injector

A simple Windows CLI program for injecting a DLL into a running process.

## Releases

Releases can be found [here](https://github.com/CrystalFerrai/DllInjector/releases). Download the zip file from a release and extract it somewhere. There is no installer.

## Antivirus Warning

Most security and antivirus software will detect this program as a potential threat because injecting a DLL into a running process is something that malware might do. If you trust the program, you will want to add an exception to your security software allowing it to exist and run.

## How to Use

The program should be run from a command prompt or batch script. It requires two arguments.
1. The name of a currently running process in which to inject a DLL.
2. The full path to the dll file that will be injected.
```
DllInjector [process name] [dll path]
```

If injection is successful, the DLL will be loaded into the target process and receive a call to its `DllMain` with the reason `DLL_PROCESS_ATTACH` from a newly created thread.

Note: If the program you want to inject is running with admin privilege, then this program must also be run with admin privilege or the injection will fail.

# Building from Source

DllInjector is built in Visual Studio 2022. If you want to build from source, here are the basic steps to follow.

1. Clone the repo.
    ```
    git clone https://github.com/CrystalFerrai/DllInjector.git
    ```
2. Open the file `DllInjector.sln` in Visual Studio.
3. Build the solution.

# Reporting Issues

If you find any problems with the program, you can [open an issue](https://github.com/CrystalFerrai/DllInjector/issues). I will look into reported issues when I find time.
