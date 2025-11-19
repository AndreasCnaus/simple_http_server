import plotly.express as px
from datetime import datetime
import pandas as pd
import sqlite3


def main():
    # Connect to the SQLite Database
    conn = sqlite3.connect("build/tracker_board.db")

    # Read the entire table into a DataFrame
    df = pd.read_sql_query("SELECT * FROM gps_data", conn)
    conn.close()
    
    print(df)

    # Convert Date-Time into timestamp
    df["timestamp"] = pd.to_datetime({
        "year": df["year"] + 2000,  # assuming 2-digit year
        "month": df["month"],
        "day": df["day"],
        "hour": df["hour"],
        "minute": df["minute"],
        "second": df["second"]
    })
    
    # Date, start and end times for the filter
    target_date_str = "2025-11-18"
    start_time_str = "12:13:51"
    end_time_str = "12:23:28"
    
    # Prepare filtering values
    target_date = pd.to_datetime(target_date_str).date()
    start_time = datetime.strptime(start_time_str, "%H:%M:%S").time()
    end_time = datetime.strptime(end_time_str, "%H:%M:%S").time()
    
    # Create the Date Mask
    mask_date = df["timestamp"].dt.date == target_date
    
    # Create Time Mask
    time_only = df["timestamp"].dt.time
    mask_time = (time_only >= start_time) & (time_only <= end_time)
    
    # Apply combined filter
    df_filtered = df.loc[mask_date & mask_time].copy()
    
    # Plot lines + points in one go
    fig = px.scatter_mapbox(
        df_filtered,
        lat="latitude",
        lon="longitude",
        hover_name="timestamp",
        title="GPS Track (Line + Points)",
        zoom=14,
        height=600
    )

    # Show both lines and markers
    fig.update_traces(mode="lines+markers",
                      marker=dict(size=8, color="red", opacity=0.9),
                      line=dict(color="blue", width=2),
                      hovertemplate=df_filtered.apply(
                          lambda row: (
                              f"Lat: {row['latitude']:.5f}<br>"
                              f"Lon: {row['longitude']:.5f}<br>"
                              f"Date: {row['timestamp'].strftime('%d.%m.%Y')}<br>"
                              f"Time: {row['timestamp'].strftime('%H:%M:%S')}<br>"
                              f"Altitude: {row['altitude']} m<br>"
                              f"Velocity: {row['speed']/100} km/h"
                          ), axis=1
                      )
    )

    # Use free OpenStreetMap tiles
    fig.update_layout(
        mapbox_style="open-street-map",
        mapbox_center={"lat": df_filtered["latitude"].mean(), "lon": df_filtered["longitude"].mean()},
        margin={"r": 0, "t": 40, "l": 0, "b": 0}
    )

    fig.show()

if __name__ == "__main__":
    main()
