<?php 
if ($_SERVER["REQUEST_METHOD"] == "POST") { 
    // Check if file was uploaded without errors 
    if (isset($_FILES["chessfile"]) && $_FILES["chessfile"]["error"] == 0) { 
          
        $file_name     = $_FILES["chessfile"]["name"]; 
        $file_type     = $_FILES["chessfile"]["type"]; 
        $file_size     = $_FILES["chessfile"]["size"]; 
        $file_tmp_name = $_FILES["chessfile"]["tmp_name"]; 
        $file_error    = $_FILES["chessfile"]["error"]; 
          
          
	echo "<html>\n<head><title>CHESS File uploaded</title></head>\n";
	echo "<body>\n";

        echo "<div style='text-align: center; background: #4CB974;  
        padding: 30px 0 10px 0; font-size: 20px; color: #fff'> 
        File Name: " . $file_name . "</div>"; 
          
        echo "<div style='text-align: center; background: #4CB974;  
        padding: 10px; font-size: 20px; color: #fff'> 
        File Type: " . $file_type . "</div>"; 
          
        echo "<div style='text-align: center; background: #4CB974;  
        padding: 10px; font-size: 20px; color: #fff'> 
        File Size: " . $file_size . "</div>"; 
          
        echo "<div style='text-align: center; background: #4CB974;  
        padding: 10px; font-size: 20px; color: #fff'> 
        File Error: " . $file_error . "</div>"; 
          
        echo "<div style='text-align: center; background: #4CB974;  
        padding: 10px; font-size: 20px; color: #fff'> 
	File Temporary Name: " . $file_tmp_name . "</div>"; 

	echo "</body>\n</html>";
          
    } 
} 
?> 
