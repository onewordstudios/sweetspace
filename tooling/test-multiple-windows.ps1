# ================
#   INSTRUCTIONS
# ================
#
# FIRST-TIME SETUP:
# First, open an administrator PowerShell window
# (right click the start button and choose "Windows PowerShell (Admin)"
# Run `set-executionpolicy remotesigned` (without quotes) to enable scripts
# If it runs now, you're done. Otherwise, you may need to copy the contents of this file,
# delete it, recreate it, and paste the contents back in
#
# NORMAL USE:
# Right click this file and choose "Run with PowerShell"

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
