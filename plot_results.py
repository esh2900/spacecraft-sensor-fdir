from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt

# Results directory
RESULTS_DIR = Path(__file__).resolve().parent / "results"

# Simulation scenarios
SCENARIOS = [
    "nominal",
    "imu_failure",
    "gnss_dropout",
    "total_gnss_failure",
    "processing_failure",
]

# Simulation duration
SIMULATION_DURATION = 10

# FDIR status colors
COLORS = {
    "HEALTHY": "green",
    "FAILED": "red",
    "UNKNOWN": "gray"
}


def plot_fdir(fdir, scenario):

    columns = [c for c in fdir.columns if c != "time_s"]

    fig, ax = plt.subplots(figsize=(12, 5))

    for i, col in enumerate(columns):
        states = fdir[col].astype("string").str.strip()

        for status, color in COLORS.items():
            mask = states.eq(status).fillna(False)

            ax.scatter(
                fdir.loc[mask, "time_s"],
                [i] * mask.sum(),
                color=color,
                marker="s",
                s=12
            )

    for status, color in COLORS.items():
        ax.scatter([], [], color=color, label=status)

    ax.set_yticks(range(len(columns)))
    ax.set_yticklabels(columns)

    ax.set_xlabel("Simulation time (s)")
    ax.set_title(f"FDIR Status - {scenario}", pad=15)

    # Fixed simulation time axis
    ax.set_xlim(0, SIMULATION_DURATION)
    ax.set_xticks(range(0, SIMULATION_DURATION + 1))

    ax.grid(alpha=0.2)
    ax.legend()

    fig.tight_layout()

    fig.savefig(
        RESULTS_DIR / f"{scenario}_fdir_plot.png",
        dpi=200
    )

    plt.close(fig)


def plot_processing(data, scenario):

    numeric = data.drop(columns=["time_s"]).apply(
        pd.to_numeric, errors="coerce"
    )

    # Measurement columns
    imu_columns = ["ax", "ay", "az", "wx", "wy", "wz"]
    gnss_columns = ["latitude", "longitude", "altitude"]

    # Validity status columns
    imu_status = "imu_valid"
    gnss_status = "gnss_valid"

    # Keep only existing columns
    imu_columns = [col for col in imu_columns if col in numeric.columns]
    gnss_columns = [col for col in gnss_columns if col in numeric.columns]

    # Add validity indicators
    if imu_status in numeric.columns:
        imu_columns.append(imu_status)

    if gnss_status in numeric.columns:
        gnss_columns.append(gnss_status)

    nrows = max(len(imu_columns), len(gnss_columns), 1)

    fig, axes = plt.subplots(
        nrows,
        2,
        figsize=(16, max(7, 2.5 * nrows)),
        sharex=True,
        squeeze=False,
        layout="constrained"
    )

    # Plot both columns
    for j, columns in enumerate([imu_columns, gnss_columns]):

        for i, col in enumerate(columns):

            ax = axes[i, j]

            if col in ["imu_valid", "gnss_valid"]:

                # Plot boolean status
                ax.step(
                    data["time_s"],
                    numeric[col],
                    where="post",
                    color="darkorange",
                    linewidth=1.5
                )

                ax.set_yticks([0, 1])
                ax.set_yticklabels(["False", "True"])
                ax.set_ylim(-0.2, 1.2)

            else:

                # Plot sensor measurements
                ax.plot(
                    data["time_s"],
                    numeric[col],
                    linewidth=1
                )

            units = {
                "latitude": "Latitude (°)",
                "longitude": "Longitude (°)",
                "altitude": "Altitude (m)",
                "ax": "ax (m/s²)",
                "ay": "ay (m/s²)",
                "az": "az (m/s²)",
                "wx": "wx (rad/s)",
                "wy": "wy (rad/s)",
                "wz": "wz (rad/s)",
                "imu_valid": "IMU Valid",
                "gnss_valid": "GNSS Valid"
            }

            ax.set_ylabel(units.get(col, col))
            ax.grid(alpha=0.3)

    # Hide unused axes
    for i in range(len(imu_columns), nrows):
        axes[i, 0].set_visible(False)

    for i in range(len(gnss_columns), nrows):
        axes[i, 1].set_visible(False)

    # Column titles
    axes[0, 0].set_title("IMU Measurements", fontsize=12)
    axes[0, 1].set_title("GNSS Measurements", fontsize=12)

    # Configure time axis
    for ax in axes.flat:
        if ax.get_visible():
            ax.set_xlim(0, SIMULATION_DURATION)
            ax.set_xticks(range(0, SIMULATION_DURATION + 1))

    # X-axis labels
    if imu_columns:
        axes[len(imu_columns) - 1, 0].set_xlabel("Simulation time (s)")

    if gnss_columns:
        axes[len(gnss_columns) - 1, 1].set_xlabel("Simulation time (s)")

    # Figure title
    fig.suptitle(
        f"DataProcessing - {scenario}",
        fontsize=14,
        fontweight="bold"
    )

    fig.savefig(
        RESULTS_DIR / f"{scenario}_processing_plot.png",
        dpi=200,
        bbox_inches="tight"
    )

    plt.close(fig)


def main():

    for scenario in SCENARIOS:

        print(f"Generating plots for {scenario}...")

        fdir_file = RESULTS_DIR / f"{scenario}_fdir_log.csv"
        processing_file = RESULTS_DIR / f"{scenario}_processing_log.csv"

        if not fdir_file.exists() or not processing_file.exists():
            print(f"Missing CSV files for {scenario}. Skipping.")
            continue

        fdir = pd.read_csv(fdir_file)
        data = pd.read_csv(processing_file)

        plot_fdir(fdir, scenario)
        plot_processing(data, scenario)

        print(f"Plots saved for {scenario}.")

    print("Plot generation completed.")


if __name__ == "__main__":
    main()
