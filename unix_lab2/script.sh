#!/bin/bash

SHARED_DIR="/program/shared"
LOCK_FILE="$SHARED_DIR/.lock"
CONTAINER_ID=$(hostname)
COUNTER=0

mkdir -p "$SHARED_DIR"

while true; do
{
    flock -x 21
    for i in $(seq -f "%03g" 1 999); do
        if [[ ! -e "$SHARED_DIR/$i" ]]; then
            ((COUNTER++))
            TMP="$SHARED_DIR/$i.tmp"
            echo "$CONTAINER_ID $COUNTER" > "$TMP"

            if ! mv "$TMP" "$SHARED_DIR/$i"; then
                echo "ERROR: failed to move $TMP to $SHARED_DIR/$i"
                rm -f "$TMP"
                continue
            fi

            echo "Created: $i by $CONTAINER_ID (#$COUNTER)"
            filename="$i"
            break
        fi
    done

} 21>"$LOCK_FILE"

sleep 1

if [[ -f "$SHARED_DIR/$filename" ]]; then
    rm -f "$SHARED_DIR/$filename"
    echo "Deleted: $filename by $CONTAINER_ID"
else
    echo "WARNING: $filename not found for deletion by $CONTAINER_ID"
fi

sleep 1
done
