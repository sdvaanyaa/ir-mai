import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


df = pd.read_csv('zipf_results.csv')

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

# Линейный масштаб 
ax1.plot(df['Rank'], df['Frequency'], marker='o', color='b', linestyle='-')
ax1.set_title('Закон Ципфа (Линейный масштаб)', fontsize=14)
ax1.set_xlabel('Ранг (Rank)', fontsize=12)
ax1.set_ylabel('Частота (Frequency)', fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.7)

for i in range(5):
    ax1.annotate(df['Word'][i], (df['Rank'][i], df['Frequency'][i]), 
                 textcoords="offset points", xytext=(0,10), ha='center')

# Логарифмический масштаб 
ax2.loglog(df['Rank'], df['Frequency'], marker='s', color='r', linestyle='-')
ax2.set_title('Закон Ципфа (Log-Log масштаб)', fontsize=14)
ax2.set_xlabel('Log Ранг', fontsize=12)
ax2.set_ylabel('Log Частота', fontsize=12)
ax2.grid(True, which="both", ls="-", alpha=0.5)


C = df['Frequency'][0]
ax2.plot(df['Rank'], C / df['Rank'], color='gray', linestyle='--', label='Идеальный закон (slope -1)')
ax2.legend()

plt.tight_layout()
plt.show()