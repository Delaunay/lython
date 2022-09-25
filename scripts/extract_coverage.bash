#!/bin/bash

set -xe

report=$1
csv=$2
template=$3
target=$4

# Save coverage evolution
SEARCH='<coverage line-rate="\([0-9.]*\)" branch-rate="\([0-9.]*\)" lines-covered="\([0-9.]*\)" lines-valid="\([0-9.]*\)" branches-covered="\([0-9.]*\)" branches-valid="\([0-9.]*\)" complexity="\([0-9.]*\)" timestamp="\([0-9.]*\)" version="\([a-zA-Z 0-9.]*\)">'

HEADER="timestamp,line-rate,branch-rate,lines-covered,lines-valid,branches-covered,branches-valid,complexity,version"
REPLACE='\8,\1,\2,\3,\4,\5,\6,\7,\9'

RESULT=$(cat $report | sed -n "s/$SEARCH/$REPLACE/p")

if [ -z "$RESULT" ]; then
    echo "Could not parse the coverge report"
    cat $report
else
    # Append the row to the coverage database
    echo $RESULT >> $csv

    # Coverage badge
    # ==============
    REPLACE='\1'
    COVERAGE=$(cat $report | sed -n "s/$SEARCH/$REPLACE/p")
    COVERAGE=$(awk "BEGIN {printf \"%.0f\n\", $COVERAGE * 100}")

    sed "s/COVERAGE_VALUE/$COVERAGE/p" $template > $target
fi
