import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent
        // Note: backgroundColor property not available in Qt Graphs 2D GraphsView
        // Using default background for now
        
        // Value axes for X and Y
        ValueAxis {
            id: axisX
            objectName: "axisX"  // For C++ findChild access
            titleText: "X"
            
            // Grid styling: subtle gray gridlines
            gridLineColor: "#E0E0E0"
            gridVisible: true
            minorGridVisible: false
            
            // Note: labelFont property not available in Qt 6.10 ValueAxis
            // Using default font styling
        }
        
        ValueAxis {
            id: axisY
            objectName: "axisY"  // For C++ findChild access
            titleText: "Y"
            
            // Grid styling: subtle gray gridlines
            gridLineColor: "#E0E0E0"
            gridVisible: true
            minorGridVisible: false
            
            // Note: labelFont property not available in Qt 6.10 ValueAxis
            // Using default font styling
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

