
SEARCH='<coverage line-rate="\([0-9.]*\)" branch-rate="\([0-9.]*\)" lines-covered="\([0-9.]*\)" lines-valid="\([0-9.]*\)" branches-covered="\([0-9.]*\)" branches-valid="\([0-9.]*\)" complexity="\([0-9.]*\)" timestamp="\([0-9.]*\)" version="\([a-zA-Z 0-9.]*\)">'

HEADER="timestamp, line-rate, branch-rate, lines-covered, lines-valid, branches-covered, branches-valid, complexity, version"
REPLACE='\8, \1, \2, \3, \4, \5, \6, \7, \9'

RESULT=$(cat $1 | sed -n "s/$SEARCH/$REPLACE/p")

# Append the row to the coverage database
echo $RESULT >> $2

# REPLACE='\1'
# COVERAGE=$(cat $1 | sed -n "s/$SEARCH/$REPLACE/p")
# sed -i "s//$REPLACE/" > badge.svg

