﻿# Clean output folder (delete if exists)
Remove-Item windows-build -Recurse -ErrorAction Ignore

# Create fresh output folder
New-Item -ItemType directory -Path windows-build

# Copy DLLs
Copy-Item -Path ..\build-win10\dll\x64\* -Destination windows-build

# Copy Assets
Copy-Item -Path ..\assets\* -Destination windows-build -Recurse

# Copy EXE
Copy-Item -Path ..\build-win10\x64\Debug\Sweetspace.exe -Destination windows-build

# SIG # Begin signature block
# MIIFdgYJKoZIhvcNAQcCoIIFZzCCBWMCAQExCzAJBgUrDgMCGgUAMGkGCisGAQQB
# gjcCAQSgWzBZMDQGCisGAQQBgjcCAR4wJgIDAQAABBAfzDtgWUsITrck0sYpfvNR
# AgEAAgEAAgEAAgEAAgEAMCEwCQYFKw4DAhoFAAQUOqyNuk/RInsFLwlO3Rx48bVV
# IwGgggMOMIIDCjCCAfKgAwIBAgIQFa2+C5tbV6tC87oyifpSVjANBgkqhkiG9w0B
# AQUFADAdMRswGQYDVQQDDBJMb2NhbCBDb2RlIFNpZ25pbmcwHhcNMjAwMzIxMDAy
# NDQ3WhcNMjEwMzIxMDA0NDQ3WjAdMRswGQYDVQQDDBJMb2NhbCBDb2RlIFNpZ25p
# bmcwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCh3Vpsc2CADnOU4cTV
# wOjTCp0FG4RkzAJpJbSgo6Wgnrk932gjIZtmvDrqEyPoVHv9emCM48DB1qjDY82+
# er7ykiNIcKrHeLFJQgLznnZYRbrTYkp7/1IRvHMmJ/IqakuLgB+ZLfOK9A4YR3P0
# qe1rkoDoKDXbNnmhwAb0pOYr81Ufx7gEECGhQQ27CqACs8+iPJBLLE4ZwrDd508F
# pwyTdfYquR99k94QvXSD/K2cy5ijOfzvUFVKTQFN1OZB5YBSSbxlDEtKE2Tuj47p
# 8Ff+ojRYFc7CcWdh/eX0539eBgonL2/CU+i715C0lD4Bjx4m4n8mCIESlOaE5W7M
# rvjlAgMBAAGjRjBEMA4GA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcD
# AzAdBgNVHQ4EFgQUF71kYFD9RbYtXyHjIND7ofVNNjcwDQYJKoZIhvcNAQEFBQAD
# ggEBAGHnhwKu/YrpBguZMV4rsJeccqRojjDv1YmGq/O/aMaiRbck5CAqGWgtLsre
# hP8v4NyObgzzQw0IHs8Uj2kumX1nq5+FoXhM6yBM6ujHxdcfRTrUs/Ubab/FJw2o
# aiE1uBoDU/RL6cfYiRsRMuWvWa/1fM5R9Doi58/bjXkx4Yr78ujsAfxaqxBBr6Ry
# 5u6lCA5oISTAAmXBrENuBe0XimlmXzKO/OGO2gYnPSPi4KW0hy34edSh9bvhHU4I
# MLCs/srNd0PFhMZ9YLw/xT8eDkj0xhh7m9rlO7qhWNLBQvqP8URTDJJmM0z8pAwN
# SoyNvsURMgsOH0T3Eq8sOHSXWC0xggHSMIIBzgIBATAxMB0xGzAZBgNVBAMMEkxv
# Y2FsIENvZGUgU2lnbmluZwIQFa2+C5tbV6tC87oyifpSVjAJBgUrDgMCGgUAoHgw
# GAYKKwYBBAGCNwIBDDEKMAigAoAAoQKAADAZBgkqhkiG9w0BCQMxDAYKKwYBBAGC
# NwIBBDAcBgorBgEEAYI3AgELMQ4wDAYKKwYBBAGCNwIBFTAjBgkqhkiG9w0BCQQx
# FgQU2D6S2LhxJBURBYsPbHt1v3mktfIwDQYJKoZIhvcNAQEBBQAEggEAXkjVXe0g
# TGLZlYOFD6HfaLp6aNfEEU1rXp2f6Ir1qCXuCaMqQklhsC+jc2aGocepLjJlrklK
# 5JfKia0N+7OdTbEA3E2rrzWDwH4fKEu8zPsvPRs3xYH4BrD01pe16ScHQG1z/lf9
# bsjk8PavUs4i1b9Du846xHQ5JldWZ4xtyXQBkzzMACcNOHJ77/vxSFPKf6ByF8lb
# v/6uRlI93yN/8gYjr+JovbA3pm9IMHcgvAx8eqCP7nqB372ctqhcB5/4P1ptYYZe
# oTMCFhqJ9XSHjxMu4GWDPoqgZBopGJqrgVa5+/2FyurERPfJv0aXSOiXPFv3smnr
# he4Yc3yHQUvTeA==
# SIG # End signature block
