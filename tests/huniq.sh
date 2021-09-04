#!/bin/sh

DIFF_EXEC=${DIFF_EXEC:-diff}
huniq="$1"

error=0
test() {
  local out=$("$huniq" <<<"$1")
  "$DIFF_EXEC" <(echo -nE "$out") <(echo -nE "$2") || {
    ((error+=1))
    echo With: >&2
    echo "$1" >&2
  }
}

test
test a a
test $'first\nsecond\nthird\n' $'first\nsecond\nthird'
test $'first\nsecond\nthird\nfirst' $'first\nsecond\nthird'
test $'first\nsecond\nthird\nfirst\n' $'first\nsecond\nthird'
test $'\nfirst\nfirst\n\nsecond\nsecond\n' $'\nfirst\nsecond'

exit $error
