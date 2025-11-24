import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent
        
        // Enable interactions (pan/zoom/resize)
        // Note: interactive property does not exist in Qt Graphs 6.10.1 GraphsView
        // Interactivity is enabled by default or controlled via other properties
        
        // Value axes for X and Y
        ValueAxis {
            id: axisX
            titleText: "X"
        }
        
        ValueAxis {
            id: axisY
            titleText: "Y"
        }
        
        // LineSeries with axes assigned directly (not in Component.onCompleted)
        LineSeries {
            id: lineSeries
            objectName: "mainSeries"  // For C++ findChild access
            name: "Data"
            axisX: axisX
            axisY: axisY
        }
    }
}

