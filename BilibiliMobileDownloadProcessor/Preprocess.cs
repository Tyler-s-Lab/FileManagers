Console.WriteLine("This script removes all empty cover.jpg files.");

var entryFiles = Directory.EnumerateFiles(
		@"D:\download",
		"cover.jpg",
		SearchOption.AllDirectories
	);

foreach (var item in entryFiles) {
	if (!File.Exists(item)) {
		continue;
	}

	FileInfo fileInfo = new(item);

	if(fileInfo.Length == 0) {
		Console.WriteLine($"DEL {item}");
		File.Delete(item);
	}
	else {
		Console.WriteLine($"OK {item}");
	}

}
return;
