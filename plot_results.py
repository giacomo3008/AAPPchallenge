"""Legge results: crea due figure e una tabella dei fit, senza eseguire esperimenti."""
import csv
import json
import statistics
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

folder = Path("results")
operations = ["search", "insert", "ordered_union", "split"]
methods = operations + ["split_rebuild"]
batch = {"search": 256, "insert": 64, "ordered_union": 1, "split": 1, "split_rebuild": 1}
units = {"ns": 1, "us": 1000, "ms": 1000000, "s": 1000000000}
times = {}
work = {}
fits = {}
for method in methods:
    times[method] = {}
    work[method] = {}

# 1. Leggo i tempi. Le righe aggregate non sono misure aggiuntive.
with open(folder / "timings.json") as file:
    raw = json.load(file)
for row in raw["benchmarks"]:
    parts = row["name"].split("/")
    method = parts[0]
    family = method + "/" + parts[1]
    if family not in fits:
        fits[family] = {"model": "", "coefficient": 0, "rms": 0}

    if "cpu_coefficient" in row:
        fits[family]["model"] = row["big_o"]
        fits[family]["coefficient"] = row["cpu_coefficient"] * units[row["time_unit"]] / batch[method]
    elif "rms" in row:
        fits[family]["rms"] = 100 * row["rms"]
    elif row.get("run_type") != "aggregate":
        n = int(parts[2])
        time = row["cpu_time"] * units[row["time_unit"]] / batch[method]
        if n not in times[method]:
            times[method][n] = []
        times[method][n].append(time)

# 2. Leggo i conteggi: sommo confronti, passi e scritture dei link.
with open(folder / "work.csv") as file:
    for row in csv.DictReader(file):
        method = row["operation"]
        n = int(row["n"])
        count = int(row["comparisons"]) + int(row["traversals"]) + int(row["link_updates"])
        if n not in work[method]:
            work[method][n] = []
        work[method][n].append(count / int(row["batch"]))

# 3. Salvo modello, coefficiente per operazione e RMS per ogni seed.
with open(folder / "complexity_fits.csv", "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["operation", "seed", "automatic_model",
                     "cpu_coefficient_ns_per_operation", "rms_percent"])
    for family in fits:
        parts = family.split("/")
        seed = parts[1].replace("seed_", "")
        fit = fits[family]
        writer.writerow([parts[0], seed, fit["model"], fit["coefficient"], fit["rms"]])


def make_plot(data, filename, ylabel):
    figure, axes = plt.subplots(2, 2, figsize=(10, 7), constrained_layout=True)
    for i in range(4):
        ax = axes[i // 2][i % 2]
        operation = operations[i]
        curves = [operation]
        if operation == "split":
            curves.append("split_rebuild")

        for method in curves:
            sizes = sorted(data[method])
            averages = []
            deviations = []
            for n in sizes:
                averages.append(statistics.mean(data[method][n]))
                deviations.append(statistics.pstdev(data[method][n]))
            ax.errorbar(sizes, averages, yerr=deviations, fmt="o-", capsize=3, label=method)

        ax.set_xscale("log", base=2)
        ax.set_xticks(sizes)
        ax.set_yscale("log")
        ax.set_title(operation)
        ax.set_xlabel("N (input keys)")
        ax.set_ylabel(ylabel)
        ax.grid(True, alpha=0.25)
        ax.legend()
    figure.savefig(folder / filename, dpi=180)
    plt.close(figure)


make_plot(times, "timings.png", "CPU time per operation (ns)")
make_plot(work, "logical_work.png", "Logical events per operation")
print("Creati timings.png, logical_work.png e complexity_fits.csv in results")
