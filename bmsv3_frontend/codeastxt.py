import os

# Define source directories
styles_dir = r"E:\BlockMySeat\bmsv3\bmsv3_frontend\styles"
html_js_dir = r"E:\BlockMySeat\bmsv3\bmsv3_frontend" # This is the directory containing your .html and .js files

# Define destination directory
destination_dir = r"E:\BlockMySeat\bmsv3\codeastext"

# Create the destination directory if it doesn't exist
os.makedirs(destination_dir, exist_ok=True)
print(f"Destination directory ensured: {destination_dir}")

def process_files_in_directory(source_directory, file_extensions):
    """
    Processes files in a given directory, reading their content and saving to .txt files.
    """
    print(f"\nProcessing files in: {source_directory}")
    for filename in os.listdir(source_directory):
        file_path = os.path.join(source_directory, filename)

        # Check if it's a file and matches the desired extensions
        if os.path.isfile(file_path) and any(filename.endswith(ext) for ext in file_extensions):
            try:
                with open(file_path, 'r', encoding='utf-8') as f_read:
                    content = f_read.read()

                # Determine the new filename based on the original file's extension
                base_name, original_ext = os.path.splitext(filename)
                
                # Remove the leading dot from the extension for the new name
                new_filename = f"{base_name}{original_ext[1:]}.txt"
                destination_file_path = os.path.join(destination_dir, new_filename)

                with open(destination_file_path, 'w', encoding='utf-8') as f_write:
                    f_write.write(content)
                print(f"Successfully processed: {filename} -> {new_filename}")

            except Exception as e:
                print(f"Error processing {filename}: {e}")

# Process CSS files
process_files_in_directory(styles_dir, ['.css'])

# Process HTML and JS files
# We need to filter based on the image provided. Only process directly listed files.
# The image shows HTML and JS files directly under bmsv3_frontend, not in subfolders.
html_js_files_to_process = [
    'bookings.html', 'bookings.js', 'confirmation.html', 'confirmation.js',
    'index.html', 'index.js', 'movie-details.html', 'movie-details.js',
    'movies.html', 'movies.js', 'seats.html', 'seats.js','venue-details.html', 'venue-details.js',
    'venues.html', 'venues.js'
]

print(f"\nProcessing HTML and JS files in: {html_js_dir}")
for filename in os.listdir(html_js_dir):
    if filename in html_js_files_to_process:
        file_path = os.path.join(html_js_dir, filename)
        try:
            with open(file_path, 'r', encoding='utf-8') as f_read:
                content = f_read.read()

            base_name, original_ext = os.path.splitext(filename)
            new_filename = f"{base_name}{original_ext[1:]}.txt"
            destination_file_path = os.path.join(destination_dir, new_filename)

            with open(destination_file_path, 'w', encoding='utf-8') as f_write:
                f_write.write(content)
            print(f"Successfully processed: {filename} -> {new_filename}")

        except Exception as e:
            print(f"Error processing {filename}: {e}")

print("\nScript finished.")