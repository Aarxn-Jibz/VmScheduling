```mermaid
flowchart LR
    %% All nodes have dark text (#000) for high contrast on light shapes
    
    A[(training_data.csv)] -->|Trains Decision Tree| B["train.py<br>Scikit-Learn"]
    B -->|Transpiles via m2cgen| C["model.c<br>C-Native ML Brain"]
    C -->|Linked at Compile| D{"main.c<br>SmartVM Engine"}
    D -->|Telemetry & Routing| E(("Benchmark<br>Results"))
    
    %% Colors are the same light pastels, but with black text explicitly added.
    style A fill:#f9f,color:#000,stroke:#333,stroke-width:2px;
    style B fill:#bbf,color:#000,stroke:#333,stroke-width:2px;
    style C fill:#bfb,color:#000,stroke:#333,stroke-width:2px;
    style D fill:#fbb,color:#000,stroke:#333,stroke-width:2px;
```