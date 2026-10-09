const express = require("express");
const http = require("http");
const cors = require("cors");
const mqtt = require("mqtt");
const { Server } = require("socket.io");

const app = express();
const server = http.createServer(app);

app.use(cors());
app.use(express.json());

const io = new Server(server, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    }
});

const MQTT_BROKER = "mqtt://broker.hivemq.com:1883";
const MQTT_TOPIC = "aquawatch/demo/waterquality";

const mqttClient = mqtt.connect(MQTT_BROKER);

mqttClient.on("connect", () => {
    console.log("Connected to MQTT broker");

    mqttClient.subscribe(MQTT_TOPIC, (err) => {
        if (err) {
            console.log("MQTT subscription error:", err);
        } else {
            console.log("Subscribed to:", MQTT_TOPIC);
        }
    });
});

mqttClient.on("message", (topic, message) => {
    try {
        const data = JSON.parse(message.toString());

        console.log("Received:", data);

        io.emit("sensorData", data);
    } catch (error) {
        console.log("Invalid MQTT data:", error.message);
    }
});

mqttClient.on("error", (error) => {
    console.log("MQTT Error:", error.message);
});

io.on("connection", (socket) => {
    console.log("Dashboard connected:", socket.id);

    socket.on("disconnect", () => {
        console.log("Dashboard disconnected:", socket.id);
    });
});

app.get("/", (req, res) => {
    res.send("AquaWatch backend is running");
});

server.listen(5000, () => {
    console.log("Backend running on http://localhost:5000");
});