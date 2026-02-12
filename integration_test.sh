#!/bin/bash

# Integration test for Phase 2: Metadata Editing
# This script tests the full workflow: import → add metadata → export

set -e  # Exit on error

echo "=== Phase 2 Integration Test ==="
echo ""

cd "$(dirname "$0")/build"

# Clean up previous test outputs
rm -f integration_test_*.jsonl integration_test_*.json integration_test_*.csv

echo "Step 1: Import test files..."
./test_cli ../test_data/sample1.txt ../test_data/sample2.txt ../test_data/image1.png ../test_data/image2.png

echo ""
echo "Step 2: Verify basic import worked..."
if [ -f "output.jsonl" ]; then
    echo "✓ Output file created"
    SAMPLE_COUNT=$(tail -n +2 output.jsonl | wc -l)
    echo "✓ Imported $SAMPLE_COUNT samples"
else
    echo "✗ Output file not found"
    exit 1
fi

echo ""
echo "Step 3: Run metadata editing test..."
./test_metadata

echo ""
echo "Step 4: Verify metadata was persisted..."
if [ -f "test_metadata_output.jsonl" ]; then
    echo "✓ Metadata test output created"
    
    # Check for tags
    if grep -q '"tags":\["training","validated","text"\]' test_metadata_output.jsonl; then
        echo "✓ Tags found: training, validated, text"
    else
        echo "✗ Expected tags not found"
        exit 1
    fi
    
    # Check for labels
    if grep -q '"category":"positive"' test_metadata_output.jsonl; then
        echo "✓ Label found: category=positive"
    else
        echo "✗ Expected label not found"
        exit 1
    fi
    
    if grep -q '"score":0.95' test_metadata_output.jsonl; then
        echo "✓ Label found: score=0.95"
    else
        echo "✗ Expected score label not found"
        exit 1
    fi
    
    if grep -q '"verified":true' test_metadata_output.jsonl; then
        echo "✓ Label found: verified=true"
    else
        echo "✗ Expected verified label not found"
        exit 1
    fi
    
    echo "✓ All metadata fields persisted correctly"
else
    echo "✗ Metadata test output not found"
    exit 1
fi

echo ""
echo "=== Integration Test PASSED ==="
echo ""
echo "Summary:"
echo "  - Import functionality: ✓"
echo "  - Metadata update API: ✓"
echo "  - Tags persistence: ✓"
echo "  - Labels persistence: ✓"
echo "  - Typed values (float, bool): ✓"
echo "  - JSONL export: ✓"
echo ""
echo "Phase 2 implementation is fully functional!"
