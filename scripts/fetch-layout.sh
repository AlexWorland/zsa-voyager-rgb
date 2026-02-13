#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

# Accept URL or hashId
INPUT="${1:?Usage: fetch-layout.sh <oryx-url-or-hash> [geometry]}"
GEOMETRY="${2:-voyager}"

# Extract hashId from URL if full URL provided
if [[ "$INPUT" == *"configure.zsa.io"* ]]; then
    HASH_ID=$(echo "$INPUT" | sed -n 's|.*layouts/\([^/]*\).*|\1|p')
else
    HASH_ID="$INPUT"
fi

echo "Fetching layout: $HASH_ID (geometry: $GEOMETRY)"

# Fetch layout JSON via GraphQL
RESPONSE=$(curl -s -X POST https://oryx.zsa.io/graphql \
  -H 'Content-Type: application/json' \
  -d "{\"operationName\":\"getLayout\",\"variables\":{\"hashId\":\"$HASH_ID\",\"geometry\":\"$GEOMETRY\",\"revisionId\":\"latest\"},\"query\":\"query getLayout(\$hashId: String!, \$revisionId: String!, \$geometry: String) { layout(hashId: \$hashId, geometry: \$geometry, revisionId: \$revisionId) { hashId title geometry revision { hashId qmkVersion layers { position title keys color } } } }\"}")

# Save full response
mkdir -p "$PROJECT_DIR/rgb-schemes"
echo "$RESPONSE" | python3 -m json.tool > "$PROJECT_DIR/rgb-schemes/layout.json"

# Extract revision hash for source download
REVISION_HASH=$(echo "$RESPONSE" | python3 -c "import json,sys; print(json.load(sys.stdin)['data']['layout']['revision']['hashId'])")
LAYOUT_HASH=$(echo "$RESPONSE" | python3 -c "import json,sys; print(json.load(sys.stdin)['data']['layout']['hashId'])")
TITLE=$(echo "$RESPONSE" | python3 -c "import json,sys; print(json.load(sys.stdin)['data']['layout']['title'])")

echo "Layout: $TITLE"
echo "Hash: $LAYOUT_HASH, Revision: $REVISION_HASH"
echo "Layers:"
echo "$RESPONSE" | python3 -c "
import json, sys
layers = json.load(sys.stdin)['data']['layout']['revision']['layers']
for l in layers:
    print(f\"  Layer {l['position']}: {l['title']} ({len(l['keys'])} keys)\")
"

# Download source ZIP
echo "Downloading QMK source..."
mkdir -p "$PROJECT_DIR/oryx-export"
curl -sL "https://oryx.zsa.io/$LAYOUT_HASH/$REVISION_HASH/source" -o "$PROJECT_DIR/oryx-export/source.zip"

# Unzip, overwriting
cd "$PROJECT_DIR/oryx-export"
unzip -o source.zip

echo "Done. Source files in oryx-export/, layout JSON in rgb-schemes/layout.json"
