import plotly.express as px
import pandas as pd
import sqlite3

def main():
    # Connect to the SQLite Database
    conn = sqlite3.connect("build/tracker_board.db")

    # Read the entire table into a DataFrame
    df = pd.read_sql_query("SELECT * FROM gps_data", conn)
    conn.close()

    # Convert Date-Time into timestamp
    df["timestamp"] = pd.to_datetime({
        "year": df["year"] + 2000,  # assuming 2-digit year
        "month": df["month"],
        "day": df["day"],
        "hour": df["hour"],
        "minute": df["minute"],
        "second": df["second"]
    })

    # Plot lines + points in one go
    fig = px.scatter_mapbox(
        df,
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
                      hovertemplate=df.apply(
                          lambda row: (
                              f"Lat: {row['latitude']:.5f}<br>"
                              f"Lon: {row['longitude']:.5f}<br>"
                              f"Date: {row['timestamp'].strftime('%d.%m.%Y')}<br>"
                              f"Time: {row['timestamp'].strftime('%H:%M:%S')}<br>"
                              f"Altitude: {row['altitude']} m<br>"
                              f"Velocity: {row['speed']} km/h"
                          ), axis=1
                      )
    )

    # Use free OpenStreetMap tiles
    fig.update_layout(
        mapbox_style="open-street-map",
        mapbox_center={"lat": df["latitude"].mean(), "lon": df["longitude"].mean()},
        margin={"r": 0, "t": 40, "l": 0, "b": 0}
    )

    fig.show()

if __name__ == "__main__":
    main()
