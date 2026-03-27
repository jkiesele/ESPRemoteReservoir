#pragma once
#include <Arduino.h>
#include "RemoteReservoir.h"
#include "WebDisplay.h"

// class that displays the reservoir data on a web page.
// a horizontal traffic light is used to indicate a valid connection (green), or no connection/error (red).
// then, the reservoir data is displayed next to it in one line, with the following format:
// [Reservoir Name (externally set)] [Reservoir Level (in %)] [Reservoir Volume (in liters)] [Temperature (in °C)] 
class RemoteReservoirDisplay : public WebDisplayBase {
public:
    RemoteReservoirDisplay(const RemoteReservoir& reservoir, const String &id, uint32_t updateIntervalSecs)
        : WebDisplayBase(id, updateIntervalSecs), reservoir_(reservoir) {}

    void setName(const String& name) { name_ = name; }

    String routeText() const override {
        String json;
        json.reserve(id_.length() + name_.length() + 160);
    
        json += F("{\"id\":\"");
        json += jsonEscape(id());
        json += F("\",\"value\":");
        json += F("{\"status\":\"");
        switch (reservoir_.status()) {
            case RemoteReservoir::Status::Unknown: json += F("disconnected"); break;
            case RemoteReservoir::Status::Fresh:   json += F("connected");    break;
            case RemoteReservoir::Status::Stale:   json += F("disconnected"); break;
        }
    
        float level = reservoir_.info().level();
        if (isnan(level) || isinf(level)) level = 0.0f;
    
        float capacity = reservoir_.info().capacity();
        if (isnan(capacity) || isinf(capacity)) capacity = 0.0f;
    
        float temperature = reservoir_.info().temperature();
        if (isnan(temperature) || isinf(temperature)) temperature = 0.0f;
    
        if (level < 0.0f) level = 0.0f;
        if (level > 100.0f) level = 100.0f;
    
        json += F("\",\"level\":");
        json += String(level, 1);
        json += F(",\"capacity\":");
        json += String(capacity, 1);
        json += F(",\"temperature\":");
        json += String(temperature, 1);
        json += F(",\"name\":\"");
        json += jsonEscape(name_);
        json += F("\"}}");
    
        return json;
    }

    String createHtmlFragment() const override {
        String html;
        html.reserve(2600);
    
        html += "<div id='"; html += id(); html += "_wrap' ";
        html += "style='display:flex;align-items:center;gap:0.5em;flex-wrap:wrap'>";
        
        html += "<span id='"; html += id(); html += "_name'>Loading...</span>";
        html += "<span id='"; html += id(); html += "_status'></span>";
        html += "<span id='"; html += id(); html += "_leveltxt'></span>";
    
        html += "<span style='display:inline-block;width:6em;height:0.8em;";
        html += "border:1px solid #888;border-radius:4px;overflow:hidden;";
        html += "background:#e0e0e0;vertical-align:middle'>";
        html += "<span id='"; html += id(); html += "_bar' ";
        html += "style='display:block;height:100%;width:0%;background:#4caf50'></span>";
        html += "</span>";
    
        html += "<span id='"; html += id(); html += "_cap'></span>";
        html += "<span id='"; html += id(); html += "_temp'></span>";
        html += "</div>\n";
    
        html += "<script>\n"
                "(function(){\n"
                "  const nameEl=document.getElementById('"; html += id(); html += "_name');\n"
                "  const statusEl=document.getElementById('"; html += id(); html += "_status');\n"
                "  const levelEl=document.getElementById('"; html += id(); html += "_leveltxt');\n"
                "  const capEl=document.getElementById('"; html += id(); html += "_cap');\n"
                "  const tempEl=document.getElementById('"; html += id(); html += "_temp');\n"
                "  const barEl=document.getElementById('"; html += id(); html += "_bar');\n"
                "\n"
                "  function clamp(x, lo, hi){ return Math.min(hi, Math.max(lo, x)); }\n"
                "\n"
                "  async function poll(){\n"
                "    try{\n"
                "      const r=await fetch('"; html += handle(); html += "');\n"
                "      if(!r.ok) return;\n"
                "      const d=await r.json();\n"
                "      const v=d.value || {};\n"
                "\n"
                "      const name = v.name || '';\n"
                "      const status = v.status || 'unknown';\n"
                "      const level = clamp(Number(v.level || 0), 0, 100);\n"
                "      const capacity = Number(v.capacity || 0);\n"
                "      const temperature = Number(v.temperature || 0);\n"
                "\n"
                "      nameEl.textContent = name;\n"
                "      statusEl.textContent = 'status: ' + status;\n"
                "      levelEl.textContent = 'level: ' + level.toFixed(1) + '%';\n"
                "      capEl.textContent = 'capacity: ' + capacity.toFixed(1) + ' l';\n"
                "      tempEl.textContent = 'temperature: ' + temperature.toFixed(1) + ' C';\n"
                "      barEl.style.width = level.toFixed(1) + '%';\n"
                "      barEl.style.background = (status === 'connected') ? '#4caf50' : '#9e9e9e';\n"
                "    }catch(e){ console.error(e); }\n"
                "  }\n"
                "\n"
                "  poll();\n"
                "  setInterval(poll, ";
        html += String(updateInterval() * 1000);
        html += ");\n"
                "})();\n"
                "</script>\n";
    
        return html;
    }

    void loop() {
    }

private:
    const RemoteReservoir& reservoir_;
    String name_;
};

