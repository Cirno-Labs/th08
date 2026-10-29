# orig/

Put your own copy of the game here. Nothing in this directory except this
file and the `.sha1` list is tracked.

The target is 東方永夜抄 1.00d (2004-09-19), the last update: the files
from the retail CD (1.00b), updated by the official
`eiya_update100b_100d.lzh` patch from
<https://www16.big.or.jp/~zun/html/th08dl.html>. The patch takes the game
to 1.00c and then to 1.00d, and updates `th08.exe`, `th08.dat`,
`replayview.exe` and `readme.txt`. It needs `readme.txt` in the game
directory. Copies bought at Comiket 66 are 1.00a and need
`eiya_update100a_100d.lzh` instead.

```
orig/th08_100d/            the installed game
orig/th08_100d/th08.exe    updated to 1.00d
```

```sh
sha1sum -c th08_100d.sha1
```

`th08.dat` must be the original, patched to 1.00d; English patches
replace it.
