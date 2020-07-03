use std::env;
use async_std::*;

fn main() {
	let fixup_file = |path| -> Result<_, String> {
		let exifreader = exif::Reader::new();

		//todo async bufreader
		let img = image::io::Reader::open(&path).map_err(|e| format!("Can't open file {}, {}", path, e))?;
		println!("Processing file {}", path);

		if img.format() != Some(image::ImageFormat::Jpeg) {
			return Err("Not a Jpeg file".to_string());
		};

		let exif = {
			let file = std::fs::File::open(&path).map_err(|e| format!("Can't open file {}, {}", path, e))?;
			let mut exif_buf = std::io::BufReader::new(&file);
			exifreader.read_from_container(&mut exif_buf).map_err(|e| format!("Can't read exif, {}", e))?
		};

		let field = exif.fields().find(|f| f.tag == exif::Tag::Orientation).ok_or("No orientation in exif")?;
		let rotation = field.value.get_uint(0).ok_or("Invalid data in orientation exif field")?;
		println!("Rotation, {}", rotation);

		if (3..=8).contains(&rotation) {
			let img = img.decode().map_err(|e| format!("Can't decode image, {}", e))?;
			let img = match rotation {
				3 | 4 => img.rotate180(),
				5 | 6 => img.rotate90(),
				7 | 8 => img.rotate270(),
				v => return Err(format!("Image has invalid orientation {}", v)),
			};
			println!("Normalized {}", path);

			//save temp and move in case program terminates mid-write
			let tmp = format!("_{}.tmp.jpg", path);
			img.save(&tmp).map_err(|e| format!("Can't save image, {}", e))?;
			std::fs::rename(&tmp, &path).map_err(|e| format!("Can't overwrite original file, {}", e))?;
		}
		Ok(())
	};

	let tasks = env::args().skip(1).map(|path| {
		task::spawn(async move {
			if let Err(e) = fixup_file(path) {
				eprintln!("{}", e);
			}
		})
	});

	task::block_on(async move {
		for task in tasks {
			task.await
		}
	});
}
