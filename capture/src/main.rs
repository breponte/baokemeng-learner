use fs_extra::dir;
use std::time::Instant;
use xcap::Monitor;

/*
 * Normalize the filename by removing characters that are not allowed in file names on Windows. 
 */
fn normalized(filename: String) -> String {
    filename.replace(['|', '\\', ':', '/'], "")
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Get all monitors and create the target directory for saving images
    let monitors = Monitor::all()?;
    dir::create_all("target/captures", true).unwrap();
    
    // Find the primary monitor
    let monitor = monitors
        .into_iter()
        .find(|m| m.is_primary().unwrap_or(false))
        .expect("No primary monitor found");

    // Get the width and height of the primary monitor
    let monitor_width = monitor.width()?;
    let monitor_height = monitor.height()?;

    // Print the primary monitor's friendly name and its resolution
    // println!(
    //     "Primary monitor: {} ({}x{})",
    //     monitor.friendly_name().unwrap(),
    //     monitor_width,
    //     monitor_height
    // );

    // Define captured region's dimensions
    let region_width = 960u32;
    let region_height = 540u32;

    // Calculate top-left corner of captured region
    let x = ((monitor_width as u32) - (region_width as u32)) / 2;
    let y = ((monitor_height as u32) - (region_height as u32)) / 2;

    let start = Instant::now();
    // Capture the region of the primary monitor
    let image = monitor.capture_region(x, y, region_width, region_height)?;
    println!(
        "Time to record region of size {}x{}: {:?}",
        image.width(),
        image.height(),
        start.elapsed()
    );

    // Save the captured region to a file
    image
        .save(format!(
            "target/captures/monitor-{}-region.png",
            normalized(monitor.friendly_name().unwrap())
        ))
        .unwrap();

    Ok(())
}
