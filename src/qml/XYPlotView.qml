import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"  // For C++ findChild access
        anchors.fill: parent
        
        // Chart configuration for 2D XY line plot
        LineSeries {
            id: lineSeries
            objectName: "mainSeries"  // For C++ findChild access
            name: "Data"
        }
        
        // Value axes for X and Y
        ValueAxis {
            id: axisX
            titleText: "X"
        }
        
        ValueAxis {
            id: axisY
            titleText: "Y"
        }
        
        // Set axes on the series
        Component.onCompleted: {
            lineSeries.axisX = axisX
            lineSeries.axisY = axisY
        }
    }
}

