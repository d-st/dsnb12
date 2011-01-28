#!/bin/sh

../spl $1 o.my
./spl-reference $1 o.ref
diff -u o.my o.ref > o.diff
less o.diff
rm o.*
