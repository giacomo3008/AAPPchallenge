"""Facoltativo: avvia i due programmi gia' compilati, dalla cartella del progetto."""
import subprocess
from pathlib import Path

Path("results").mkdir(exist_ok=True)

print("1/2: conteggi del lavoro...", flush=True)
subprocess.run(["./work_counts", "results/work.csv"], check=True)

print("2/2: misure dei tempi...", flush=True)
with open("results/benchmark_console.txt", "w") as log:
    subprocess.run(["./benchmarks", "--benchmark_out=results/timings.json",
                    "--benchmark_out_format=json", "--benchmark_color=false"],
                   stdout=log, stderr=subprocess.STDOUT, check=True)

print("Finito. Per i grafici: python3 plot_results.py")
