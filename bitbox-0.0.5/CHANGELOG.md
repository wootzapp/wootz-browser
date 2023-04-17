## [0.0.1] - 2019-08-01

Light version of Bitcoin.com's Bitbox library

## [0.0.2] - 2019-10-06

- Taken some code analysis hint into account

## [0.0.3] - 2020-03-20

- Added support for bip21 implemented by RomitRadical
- Added transaction details support
- Fixed some API issues and

## [0.0.4] - 2020-05-12

- padded a private key by if it was generated shorter than 32 bytes
- added support for broadcasting more transactions
- changed resturl parameter to non-named (didn't make sense to have it named when it was the only one))

## [0.0.5] - 2020-06-02

- fixed signing mechanism, so it would better handle 31-byte private key
