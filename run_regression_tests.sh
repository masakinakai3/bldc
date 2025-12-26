#!/bin/bash
# =============================================================================
# VESC Firmware Regression Test Runner
# 
# This script automates the Phase 5 regression test workflow:
# 1. Build Phase 5 components
# 2. Run all Phase 5 tests
# 3. Generate plots from results
# 4. Open verification report in VS Code
# =============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "VESC Firmware Regression Test Runner"
echo "========================================"
echo ""

# Step 1: Navigate to pc_build directory
echo -e "${YELLOW}[1/5]${NC} Navigating to pc_build directory..."
cd pc_build || exit 1
echo -e "${GREEN}✓${NC} Done"
echo ""

# Step 2: Build Phase 5 components
echo -e "${YELLOW}[2/5]${NC} Building Phase 5 components (make phase5)..."
make phase5
echo -e "${GREEN}✓${NC} Build complete"
echo ""

# Step 3: Run all Phase 5 tests
echo -e "${YELLOW}[3/5]${NC} Running Phase 5 tests (make test_phase5)..."
make test_phase5
echo -e "${GREEN}✓${NC} All tests complete"
echo ""

# Step 4: Generate plots
echo -e "${YELLOW}[4/5]${NC} Generating plots (python3 plot_regression_results.py)..."
python3 plot_regression_results.py
echo -e "${GREEN}✓${NC} Plots generated in results/plots/"
echo ""

# Step 5: Open verification report in VS Code
echo -e "${YELLOW}[5/5]${NC} Opening verification report..."
cd ..
code doc/design/test/phase6_verification_report.md
echo -e "${GREEN}✓${NC} Report opened in VS Code"
echo ""

echo "========================================"
echo -e "${GREEN}Regression test workflow complete!${NC}"
echo "========================================"
echo ""
echo "Results location:"
echo "  - CSV data:  pc_build/results/*.csv"
echo "  - Plots:     pc_build/results/plots/*.png"
echo "  - Report:    doc/design/test/phase6_verification_report.md"
echo ""
