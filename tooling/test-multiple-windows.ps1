# Clean output folder (delete if exists)
Remove-Item windows-build -Recurse -ErrorAction Ignore

# Create fresh output folder
New-Item -ItemType directory -Path windows-build

# Copy DLLs
Copy-Item -Path ..\build-win10\dll\x64\* -Destination windows-build

# Copy Assets
Copy-Item -Path ..\assets\* -Destination windows-build -Recurse

# Copy EXE
Copy-Item -Path ..\build-win10\x64\Debug\Sweetspace.exe -Destination windows-build
