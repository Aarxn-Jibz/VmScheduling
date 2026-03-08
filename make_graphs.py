import pandas as pd
import matplotlib.pyplot as plt
from sklearn.tree import DecisionTreeClassifier, plot_tree

# --- 1. Generate the Scatter Plot ---
print("Generating training data distribution...")
df = pd.read_csv("training_data.csv")
colors = df["Category"].map({0: "steelblue", 1: "tomato"})

plt.figure(figsize=(8, 6))
plt.scatter(df["CPU_Ops"], df["IO_Wait"], c=colors, alpha=0.6, edgecolors="k")
plt.xlabel("CPU Operations")
plt.ylabel("I/O Wait Operations")
plt.title("Training Data Distribution\n(Blue = CPU-Heavy, Red = I/O-Heavy)")
plt.grid(True, linestyle="--", alpha=0.5)
plt.savefig("training_data_distribution.png", dpi=150, bbox_inches="tight")
plt.clf()  # Clear the canvas

# --- 2. Generate the Decision Tree Visual ---
print("Generating decision tree visual...")
X = df[["CPU_Ops", "IO_Wait"]]
y = df["Category"]

# Re-train the exact same tree to visualize it
clf = DecisionTreeClassifier(max_depth=3, random_state=42)
clf.fit(X, y)

plt.figure(figsize=(12, 6))
plot_tree(
    clf,
    feature_names=["CPU_Ops", "IO_Wait"],
    class_names=["CPU-Heavy", "IO-Heavy"],
    filled=True,
    rounded=True,
)
plt.title("Trained ML Scheduler Decision Tree")
plt.savefig("decision_tree_visual.png", dpi=150, bbox_inches="tight")
plt.clf()

# --- 3. Generate the Scheduler Comparison Bar Chart ---
print("Generating scheduler comparison chart...")
schedulers = [
    "Round Robin\n(Dumb)",
    "MLFQ\n(Industry Standard)",
    "AI-Augmented\n(Smart)",
]
turnaround_times = [46538, 39396, 37646]
bar_colors = ["#cccccc", "#8ebad9", "#2ca02c"]  # Grey, Light Blue, Green for the winner

plt.figure(figsize=(8, 6))
bars = plt.bar(schedulers, turnaround_times, color=bar_colors, edgecolor="black")
plt.ylabel("Average Turnaround Time (CPU Cycles)")
plt.title("Scheduler Performance Comparison (Lower is Better)")

# Add the exact numbers on top of the bars
for bar in bars:
    yval = bar.get_height()
    plt.text(
        bar.get_x() + bar.get_width() / 2,
        yval + 500,
        f"{int(yval)}",
        ha="center",
        va="bottom",
        fontweight="bold",
    )

plt.ylim(0, 55000)  # Give some headroom for the text
plt.savefig("scheduler_comparison_chart.png", dpi=150, bbox_inches="tight")
print("Done! All three images have been saved to your folder.")
