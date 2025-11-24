import QtQuick 2.15
import QtGraphs 6.10

Item {
    id: root

    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent

        // QtGraphs 6.10 interaction API
        zoomStyle: GraphsView.ZoomStyle.Center
        panStyle: GraphsView.PanStyle.Drag

        ValueAxis {
            id: axisX
            objectName: "axisX"
            titleText: "X"
            gridVisible: true
        }

        ValueAxis {
            id: axisY
            objectName: "axisY"
            titleText: "Y"
            gridVisible: true
        }

        LineSeries {
            id: lineSeries
            objectName: "mainSeries"
            name: "Data"
            axisX: axisX
            axisY: axisY
            color: "#2196F3"
            width: 2.0
        }
    }
}
