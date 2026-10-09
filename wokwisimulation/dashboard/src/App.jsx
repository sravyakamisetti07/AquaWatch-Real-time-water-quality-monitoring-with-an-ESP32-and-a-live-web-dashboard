import React, { useEffect, useState } from "react";
import { io } from "socket.io-client";

const socket = io("http://localhost:5000");

function App() {
    const [data, setData] = useState({
        ph: 0,
        turbidity: 0,
        tds: 0,
        status: "WAITING"
    });

    const [connected, setConnected] = useState(false);

    useEffect(() => {
        socket.on("connect", () => {
            setConnected(true);
        });

        socket.on("disconnect", () => {
            setConnected(false);
        });

        socket.on("sensorData", (newData) => {
            setData(newData);
        });

        return () => {
            socket.off("connect");
            socket.off("disconnect");
            socket.off("sensorData");
        };
    }, []);

    const ph = Number(data.ph) || 0;
    const turbidity = Number(data.turbidity) || 0;
    const tds = Number(data.tds) || 0;

    const phSafe = ph >= 6.5 && ph <= 8.5;
    const turbiditySafe = turbidity <= 5;
    const tdsSafe = tds <= 500;

    const safe = phSafe && turbiditySafe && tdsSafe;

    return (
        <div className="app">

            <header>
                <div>
                    <h1>💧 AquaWatch</h1>
                    <p>Real-Time Water Quality Monitoring</p>
                </div>

                <div
                    className={
                        connected
                            ? "connection online"
                            : "connection offline"
                    }
                >
                    <span></span>
                    {connected
                        ? "Dashboard Connected"
                        : "Disconnected"}
                </div>
            </header>

            <main>

                <section className="intro">
                    <h2>Water Quality Dashboard</h2>
                    <p>
                        Live monitoring of pH, turbidity and TDS values
                    </p>
                </section>

                <section className="cards">

                    <div className="card">
                        <h3>🧪 pH Level</h3>

                        <div className="value">
                            {ph.toFixed(2)}
                        </div>

                        <p>Safe range: 6.5 – 8.5</p>

                        <div
                            className={
                                phSafe
                                    ? "safe badge"
                                    : "alert badge"
                            }
                        >
                            {phSafe ? "SAFE" : "ALERT"}
                        </div>
                    </div>

                    <div className="card">
                        <h3>💧 Turbidity</h3>

                        <div className="value">
                            {turbidity.toFixed(1)}
                        </div>

                        <p>Limit: ≤ 5 NTU</p>

                        <div
                            className={
                                turbiditySafe
                                    ? "safe badge"
                                    : "alert badge"
                            }
                        >
                            {turbiditySafe ? "SAFE" : "ALERT"}
                        </div>
                    </div>

                    <div className="card">
                        <h3>🔬 TDS</h3>

                        <div className="value">
                            {tds.toFixed(1)}
                        </div>

                        <p>Limit: ≤ 500 ppm</p>

                        <div
                            className={
                                tdsSafe
                                    ? "safe badge"
                                    : "alert badge"
                            }
                        >
                            {tdsSafe ? "SAFE" : "ALERT"}
                        </div>
                    </div>

                </section>

                <section
                    className={
                        safe
                            ? "overall safe-box"
                            : "overall alert-box"
                    }
                >
                    <h2>
                        {safe
                            ? "✓ WATER QUALITY SAFE"
                            : "⚠ WATER QUALITY ALERT"}
                    </h2>

                    <p>
                        {safe
                            ? "All monitored parameters are within the defined limits."
                            : "One or more water-quality parameters have crossed the safe limit."}
                    </p>
                </section>

                <section className="live-section">

                    <h2>Live Sensor Data</h2>

                    <div className="data-table">

                        <div className="row heading">
                            <span>Parameter</span>
                            <span>Live Value</span>
                            <span>Status</span>
                        </div>

                        <div className="row">
                            <span>pH</span>
                            <span>{ph.toFixed(2)}</span>
                            <span>
                                {phSafe ? "Safe" : "Alert"}
                            </span>
                        </div>

                        <div className="row">
                            <span>Turbidity</span>
                            <span>
                                {turbidity.toFixed(1)} NTU
                            </span>
                            <span>
                                {turbiditySafe ? "Safe" : "Alert"}
                            </span>
                        </div>

                        <div className="row">
                            <span>TDS</span>
                            <span>
                                {tds.toFixed(1)} ppm
                            </span>
                            <span>
                                {tdsSafe ? "Safe" : "Alert"}
                            </span>
                        </div>

                    </div>

                </section>

            </main>

        </div>
    );
}

export default App;