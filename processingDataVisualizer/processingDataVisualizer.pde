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
String serialPortName = "COM8";
int    baud_rate      = 9600;
byte[] inBuffer       = new byte[100];
int    i           = 0;

// Used for Panel Interfacing
ControlP5 cp5;

// Settings for Plotter as saved in this file
JSONObject plotterConfigJSON;
String     topSketchPath = "";

// List of Plot Positions
int[] angle1_pos = {65,50};
int[] angle2_pos = {445,50};
int[] angle3_pos = {1585,50};
int[] angle4_pos = {1205,50};
int[] angle5_pos = {825,50};
int[] angle6_pos = {65,320};

//List of LineGraphs
Graph  angle1_linegraph  = new Graph(angle1_pos[0], angle1_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  angle2_linegraph  = new Graph(angle2_pos[0], angle2_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  angle3_linegraph  = new Graph(angle3_pos[0], angle3_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  angle4_linegraph  = new Graph(angle4_pos[0], angle4_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  angle5_linegraph  = new Graph(angle5_pos[0], angle5_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  angle6_linegraph  = new Graph(angle6_pos[0], angle6_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));

// Data Matrix for each LineGraph (rows are for each line graph within the single graph, columns are the datapoints)
float[][] angle1_linegraph_values = new float[1][50];
float[][] angle2_linegraph_values = new float[1][50];
float[][] angle3_linegraph_values = new float[1][50];
float[][] angle4_linegraph_values = new float[1][50];
float[][] angle5_linegraph_values = new float[1][50];
float[][] angle6_linegraph_values = new float[1][50];

// Sample Data
float[]   linegraph_sample = new float[50];

// Graph Colors
color[] graphColors = new color[3];




void setup()
{
  graphColors[0] = color(131, 255, 20); //GREEN
  graphColors[1] = color(147, 39, 143); //PURPLE
  graphColors[2] = color(193, 39, 45); //RED
  
  frame.setTitle("Quasar_ControlPanel");
  size(1905,1000);
  
  // Settings Save File
  topSketchPath      = sketchPath();
  plotterConfigJSON  = loadJSONObject(topSketchPath + "/plotter_config.json");
  
  // Setup ControlP5
  cp5 = new ControlP5(this);

  // Clearing each Data Matrix for the Line Graphs
  clearAllDataValues();

  // Starting Serial Communications
  serialPort = new Serial(this, serialPortName, baud_rate);

   // Creating the Textfields for Graph Range modification
   createMinMaxRange();
   
   //Setup Parameters of Each Line Graph
   setChartSettings();
}



//
void draw()
{
  if( serialPort.available() > 0 )
  {
    String myString = "";

    // Reads from the Serial Monitor and pull a single string data packet
    try {serialPort.readBytesUntil('\n', inBuffer);}
    catch(Exception e)
    {
      myString = new String(inBuffer);
    }
    myString = new String(inBuffer);


    // Parse and Update Sensor Values
    parseAndUpdateData(myString);

    // Updating Each of the Line Graph matrixs
    updateEachLineGraphMatrix();
  }


  // Set a Dark-Grey Background
  background(30); 
  
  
  // Update Sensor Value Texts
   updateSensorText();
   
  // Draw Each Axis of the Line Graphs
  drawEachLineGraphAxis();

  // Plot Each Graph
  plotEachGraph();
}


//
void updateSensorText()
{
  fill(40); color(0);stroke(131, 255, 20);strokeWeight(2);
  rect(1120,430,100,25, 10.0); //Load Cell BOX
  textSize(12);stroke(225);fill(225); 
  text(angle_data[0] + " degrees", 1210, 448);
  
  fill(40); color(0);stroke(131, 255, 20);strokeWeight(2);
  rect(390,520,100,25, 10.0); //Load Cell BOX
  textSize(12);stroke(225);fill(225); 
  text(angle_data[1] + " degrees", 480, 538);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(390,375,100,25, 10.0); //OTC1
  textSize(12);stroke(225);fill(225); 
  text(angle_data[2] + " degrees", 480, 393);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(1390,500,100,25, 10.0); //OTC3
  textSize(12);stroke(225);fill(225); 
  text(angle_data[3] + " degrees", 1480, 518);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(1240,410,100,25, 10.0); //OTC2
  textSize(12);stroke(225);fill(225); 
  text(angle_data[4] + " degrees", 1330, 428);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(1275,525,100,25, 10.0); //OTC4
  textSize(12);stroke(225);fill(225); 
  text(angle_data[5] + " degrees", 1365, 543);
}



// Setting all of the line graph settings
void setChartSettings()
{
  //Angle 1
  angle1_linegraph.xLabel=" Time(sec) ";
  angle1_linegraph.yLabel="Angle(degrees)";
  angle1_linegraph.Title="Angle 1";  
  angle1_linegraph.xDiv=7;  
  angle1_linegraph.xMax=0; 
  angle1_linegraph.yMax=int(getPlotterConfigString("angle1mx")); 
  angle1_linegraph.yMin=int(getPlotterConfigString("angle1mn"));

  //Angle 2
  angle2_linegraph.xLabel=" Time(sec) ";
  angle2_linegraph.yLabel="Angle(degrees)";
  angle2_linegraph.Title="Angle 2";  
  angle2_linegraph.xDiv=7;  
  angle2_linegraph.xMax=0; 
  angle2_linegraph.yMax=int(getPlotterConfigString("angle2mx")); 
  angle2_linegraph.yMin=int(getPlotterConfigString("angle2mn"));

  //Angle 3
  angle3_linegraph.xLabel=" Time(sec) ";
  angle3_linegraph.yLabel="Angle(degrees)";
  angle3_linegraph.Title="Angle 3";  
  angle3_linegraph.xDiv=7;  
  angle3_linegraph.xMax=0; 
  angle3_linegraph.yMax=int(getPlotterConfigString("angle3mx")); 
  angle3_linegraph.yMin=int(getPlotterConfigString("angle3mn"));
  
  //Angle 4
  angle4_linegraph.xLabel=" Time(sec) ";
  angle4_linegraph.yLabel="Angle(degrees)";
  angle4_linegraph.Title="Angle 4";  
  angle4_linegraph.xDiv=7;  
  angle4_linegraph.xMax=0; 
  angle4_linegraph.yMax=int(getPlotterConfigString("angle4mx")); 
  angle4_linegraph.yMin=int(getPlotterConfigString("angle4mn"));
  
  //Angle 5
  angle5_linegraph.xLabel=" Time(sec) ";
  angle5_linegraph.yLabel="Angle(degrees)";
  angle5_linegraph.Title="Angle 5";  
  angle5_linegraph.xDiv=7;  
  angle5_linegraph.xMax=0; 
  angle5_linegraph.yMax=int(getPlotterConfigString("angle5mx")); 
  angle5_linegraph.yMin=int(getPlotterConfigString("angle5mn"));
  
  //Angle 6
  angle6_linegraph.xLabel=" Time(sec) ";
  angle6_linegraph.yLabel="Angle(degrees)";
  angle6_linegraph.Title="Angle 6";  
  angle6_linegraph.xDiv=7;  
  angle6_linegraph.xMax=0; 
  angle6_linegraph.yMax=int(getPlotterConfigString("angle6mx")); 
  angle6_linegraph.yMin=int(getPlotterConfigString("angle6mn"));
}



// handle gui actions
void controlEvent(ControlEvent theEvent) 
{
  if (theEvent.isAssignableFrom(Textfield.class) || theEvent.isAssignableFrom(Toggle.class) || theEvent.isAssignableFrom(Button.class)) {
    String parameter = theEvent.getName();
    String value = "";
    if (theEvent.isAssignableFrom(Textfield.class))
      value = theEvent.getStringValue();
    else if (theEvent.isAssignableFrom(Toggle.class) || theEvent.isAssignableFrom(Button.class))
      value = theEvent.getValue()+"";

    plotterConfigJSON.setString(parameter, value);
    saveJSONObject(plotterConfigJSON, topSketchPath+"/plotter_config.json");
  }
  setChartSettings();
}



// get gui settings from settings file
String getPlotterConfigString(String id) 
{
  String r = "";
  try {
    r = plotterConfigJSON.getString(id);
  } 
  catch (Exception e) {
    r = "";
  }
  return r;
}



//Plotting each of the graphs
void plotEachGraph()
{
  //Angle 1
  for(int i = 0; i < angle1_linegraph_values.length; i++)
  {
    angle1_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    angle1_linegraph.LineGraph(linegraph_sample, angle1_linegraph_values[i]);
  }

  //Angle 2
  for(int i = 0; i < angle2_linegraph_values.length; i++)
  {
    angle2_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    angle2_linegraph.LineGraph(linegraph_sample, angle2_linegraph_values[i]);
  }

  //Angle 3
  for(int i = 0; i < angle3_linegraph_values.length; i++)
  {
    angle3_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    angle3_linegraph.LineGraph(linegraph_sample, angle3_linegraph_values[i]);
  }

  //Angle 4
  for(int i = 0; i < angle4_linegraph_values.length; i++)
  {
    angle4_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    angle4_linegraph.LineGraph(linegraph_sample, angle4_linegraph_values[i]);
  }
  
  //Angle 5
  for(int i = 0; i < angle5_linegraph_values.length; i++)
  {
    angle5_linegraph.GraphColor = graphColors[0]; // Set index to i if you plan on using more colors
    angle5_linegraph.LineGraph(linegraph_sample, angle5_linegraph_values[i]);
  }
  
  //Angle 6
  for(int i = 0; i < angle6_linegraph_values.length; i++)
  {
    angle6_linegraph.GraphColor = graphColors[0]; // Set index to i if you plan on using more colors
    angle6_linegraph.LineGraph(linegraph_sample, angle6_linegraph_values[i]);
  }
}



//
void drawEachLineGraphAxis()
{
  angle1_linegraph.DrawAxis();
  angle2_linegraph.DrawAxis();
  angle3_linegraph.DrawAxis();
  angle4_linegraph.DrawAxis();
  angle5_linegraph.DrawAxis();
  angle6_linegraph.DrawAxis();
}



//
void updateEachLineGraphMatrix()
{
  // Updating Angle 1
  try
  {
    for( i = 0; i < angle1_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < angle1_linegraph_values[i].length-1; col++)
      {
        angle1_linegraph_values[i][col] = angle1_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      angle1_linegraph_values[i][angle1_linegraph_values[i].length-1] = angle_data[0];
    }
  }catch(Exception e){System.out.println("Angle 1 Failed to update!");}  
  
  // Updating Angle 2
  try
  {
    for( i = 0; i < angle2_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < angle2_linegraph_values[i].length-1; col++)
      {
        angle2_linegraph_values[i][col] = angle2_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      angle2_linegraph_values[i][angle2_linegraph_values[i].length-1] = angle_data[1];
    }
  }catch(Exception e){System.out.println("Angle 2 Failed to update!");}  
  
  // Updating Angle 3
  try
  {
    for( i = 0; i < angle3_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < angle3_linegraph_values[i].length-1; col++)
      {
        angle3_linegraph_values[i][col] = angle3_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      angle3_linegraph_values[i][angle3_linegraph_values[i].length-1] = angle_data[2];
    }
  }catch(Exception e){System.out.println("Angle 3 Failed to update!");}

  // Updating Angle 4
  try
  {
    for( i = 0; i < angle4_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < angle4_linegraph_values[i].length-1; col++)
      {
        angle4_linegraph_values[i][col] = angle4_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      angle4_linegraph_values[i][angle4_linegraph_values[i].length-1] = angle_data[3];
    }
  }catch(Exception e){System.out.println("Angle 4 Failed to update!");}  
  
  // Updating Angle 5 Graph
  try
  {
    for( i = 0; i < angle5_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < angle5_linegraph_values[i].length-1; col++)
      {
        angle5_linegraph_values[i][col] = angle5_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      angle5_linegraph_values[i][angle5_linegraph_values[i].length-1] = angle_data[4];
    }
  }catch(Exception e){System.out.println("Angle 5 Failed to update!");}  
  
  // Updating Angle 6 Graph
  try
  {
    for( i = 0; i < angle6_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < angle6_linegraph_values[i].length-1; col++)
      {
        angle6_linegraph_values[i][col] = angle6_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      angle6_linegraph_values[i][angle6_linegraph_values[i].length-1] = angle_data[5];
    }
  }catch(Exception e){System.out.println("Angle 6 Failed to update!");}  
}



//Parses all of the Serial Data into the proper locations
void parseAndUpdateData(String myString)
{
  String currentString = myString;
    String subarray = "";
    
    try
    {
      subarray = currentString.substring(myString.indexOf("{")+1, myString.indexOf(","));
    }
    catch (Exception e)
    {
      System.out.println("A data packet was unable to be parsed!");
      return;
    }
  //Serial.print(subarray);
    angle_data[0] = Float.parseFloat(subarray);
    currentString = currentString.substring(myString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    angle_data[1] = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    angle_data[2] = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    angle_data[3] = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
  //-------------------------------------------------------------
    subarray = currentString.substring(0, currentString.indexOf(","));
    angle_data[4] = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf("}"));
    angle_data[5] = Float.parseFloat(subarray);
}



//Sets the current Minimum and Maximum ranges for each line graph
void createMinMaxRange()
{
    cp5.addTextfield("angle1mx").setPosition(angle1_pos[0]-55, angle1_pos[1]-30).setText(getPlotterConfigString("angle1mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle1mn").setPosition(angle1_pos[0]-55, angle1_pos[1]+190).setText(getPlotterConfigString("angle1mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle2mx").setPosition(angle2_pos[0]-55, angle2_pos[1]-30).setText(getPlotterConfigString("angle2mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle2mn").setPosition(angle2_pos[0]-55, angle2_pos[1]+190).setText(getPlotterConfigString("angle2mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle3mx").setPosition(angle3_pos[0]-55, angle3_pos[1]-30).setText(getPlotterConfigString("angle3mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle3mn").setPosition(angle3_pos[0]-55, angle3_pos[1]+190).setText(getPlotterConfigString("angle3mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle4mx").setPosition(angle4_pos[0]-55, angle4_pos[1]-30).setText(getPlotterConfigString("angle4mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle4mn").setPosition(angle4_pos[0]-55, angle4_pos[1]+190).setText(getPlotterConfigString("angle4mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle5mx").setPosition(angle5_pos[0]-55, angle5_pos[1]-30).setText(getPlotterConfigString("angle5mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle5mn").setPosition(angle5_pos[0]-55, angle5_pos[1]+190).setText(getPlotterConfigString("angle5mn")).setWidth(40).setAutoClear(false);    
    cp5.addTextfield("angle6mx").setPosition(angle6_pos[0]-55, angle6_pos[1]-30).setText(getPlotterConfigString("angle6mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("angle6mn").setPosition(angle6_pos[0]-55, angle6_pos[1]+190).setText(getPlotterConfigString("angle6mn")).setWidth(40).setAutoClear(false);
}



//Clears all of the Data Matrixes for each line graph
void clearAllDataValues()
{
  // Clearing angle 1
  for(int row = 0; row < angle1_linegraph_values.length; row++)
  {
    for (int col = 0; col < angle1_linegraph_values[0].length ; col++) {
      
      if(row == 0) {linegraph_sample[col] = col;}
      else
      {
        angle1_linegraph_values[row][col] = 0;
      }
    }
  }

  // Clearing angle 2
  for(int row = 0; row < angle2_linegraph_values.length; row++)
  {
    for (int col = 0; col < angle2_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        angle2_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing angle3
  for(int row = 0; row < angle3_linegraph_values.length; row++)
  {
    for (int col = 0; col < angle3_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        angle3_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing angle4
  for(int row = 0; row < angle4_linegraph_values.length; row++)
  {
    for (int col = 0; col < angle4_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        angle4_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing angle5
  for(int row = 0; row < angle5_linegraph_values.length; row++)
  {
    for (int col = 0; col < angle5_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        angle5_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing angle6
  for(int row = 0; row < angle6_linegraph_values.length; row++)
  {
    for (int col = 0; col < angle6_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        angle6_linegraph_values[row][col] = 0;
      //}
    }
  }
}
