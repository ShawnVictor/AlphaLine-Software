/*
 * AlphaLine_DataVisualize.ino
 * Code by: Shawn Victor
 * Last Modified: 2/3/2020 
 */
 
 //Importing Libraries
import java.awt.Frame;
import java.awt.BorderLayout;
import javax.swing.JLabel;
import javax.swing.ImageIcon;
import controlP5.*;
import processing.serial.*;


//Data 
float angle_data[] = new float[6];


// Linegraph Recommended Paramters
int linegraph_width  = 300;
int linegraph_height = 175;


// Serial Port Parameters
Serial serialPort;
String serialPortName = "COM6";
int    baud_rate      = 9600;
byte[] inBuffer       = new byte[100];
int    i           = 0;

// Used for Panel Interfacing
ControlP5 cp5;

// Settings for Plotter as saved in this file
JSONObject plotterConfigJSON;
String     topSketchPath = "";
