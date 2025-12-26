#!/usr/bin/env python3
"""
plot_regression_results.py

Generate plots from CSV results produced by Phase5 regression tests.

Produces PNG files into `results/plots/` for each test CSV found.
"""
import os
import csv
import math
from pathlib import Path

OUT_DIR = Path("results/plots")
OUT_DIR.mkdir(parents=True, exist_ok=True)

def read_csv(path):
    rows = []
    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for r in reader:
            rows.append({k: float(v) for k, v in r.items()})
    return rows

def try_import_matplotlib():
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        return plt
    except Exception as e:
        print("matplotlib is not available:", e)
        return None

def plot_time_series(rows, fields, title, outpath):
    plt = try_import_matplotlib()
    if plt is None:
        raise RuntimeError('matplotlib required to generate plots')

    times = [r['time'] for r in rows]
    fig, ax = plt.subplots(len(fields), 1, figsize=(8, 2.5 * max(1, len(fields))))
    if len(fields) == 1:
        ax = [ax]

    for i, fld in enumerate(fields):
        vals = [r.get(fld, math.nan) for r in rows]
        ax[i].plot(times, vals, '-', linewidth=1)
        ax[i].set_ylabel(fld)
        ax[i].grid(True)
    ax[-1].set_xlabel('time [s]')
    fig.suptitle(title)
    fig.tight_layout(rect=[0, 0.03, 1, 0.95])
    fig.savefig(outpath)
    plt.close(fig)
    print(f'Wrote {outpath}')

def main():
    csv_dir = Path('results')
    if not csv_dir.exists():
        print('No results/ directory found. Run tests first.')
        return 1

    patterns = ['speed_step_test.csv', 'torque_step_test.csv', 'hfi_test.csv']
    for p in patterns:
        path = csv_dir / p
        if not path.exists():
            print(f'Skipping missing {path}')
            continue

        try:
            rows = read_csv(path)
        except Exception as e:
            print(f'Failed to read {path}: {e}')
            continue

        if len(rows) == 0:
            print(f'No records in {path}')
            continue

        # Determine fields to plot
        keys = list(rows[0].keys())
        # Common fields: id, iq, omega_e, torque
        fields = [k for k in ['id','iq','omega_e','torque'] if k in keys]
        if not fields:
            # fallback: plot first numeric columns except time
            fields = [k for k in keys if k != 'time'][:3]

        outpath = OUT_DIR / (path.stem + '.png')
        try:
            plot_time_series(rows, fields, path.stem, str(outpath))
        except RuntimeError as e:
            print('Plot generation failed:', e)
            print('To enable plotting, install matplotlib in WSL:')
            print('  sudo apt update && sudo apt install -y python3-matplotlib')
            return 2

    print('Done')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
