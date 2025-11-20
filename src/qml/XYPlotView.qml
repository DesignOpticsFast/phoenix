import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent
        backgroundColor: "#FFFFFF"  // Pure white background for high contrast
        
        // Value axes for X and Y
        ValueAxis {
            id: axisX
            objectName: "axisX"  // For C++ findChild access
            titleText: "X"
            
            // Grid styling: subtle gray gridlines
            gridLineColor: "#E0E0E0"
            gridVisible: true
            minorGridVisible: false
            
            // Label font: readable size
            labelFont.pointSize: 10
        }
        
        ValueAxis {
            id: axisY
            objectName: "axisY"  // For C++ findChild access
            titleText: "Y"
            
            // Grid styling: subtle gray gridlines
            gridLineColor: "#E0E0E0"
            gridVisible: true
            minorGridVisible: false
            
            // Label font: readable size
            labelFont.pointSize: 10
        }
        
        // LineSeries with axes assigned directly (not in Component.onCompleted)
        LineSeries {
            id: lineSeries
            objectName: "mainSeries"  // For C++ findChild access
            name: "Data"
            axisX: axisX
            axisY: axisY
            
            // Line styling: thicker line with high-contrast color
            width: 2  // Increased from default 1px for better visibility
            color: "#2196F3"  // Material Blue - professional, high contrast
        }
    }
}

