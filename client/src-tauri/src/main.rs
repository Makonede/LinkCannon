/*
main.rs - Main Tauri Rust file.
Copyright (C) 2023  Makonede

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![feature(mutex_unlock)]

use std::io;
use std::io::{Read, Write};

use std::net::TcpStream;
use std::sync::{Arc, Mutex};
use std::thread;


#[tauri::command]
fn connect(address: String, port: u16) -> Result<(), String> {
  let stringify = |e: io::Error| e.to_string();

  let stream = TcpStream::connect((address.as_str(), port)).map_err(stringify)?;
  let stream = Arc::new(Mutex::new(stream));
  let mut buf = [0u8; 3];

  // Send the handshake
  let mut client = stream.lock().unwrap();
  client.write(&[0x4cu8, 0x43u8, 0x00u8]).map_err(stringify)?;
  client.read_exact(&mut buf).map_err(stringify)?;
  Mutex::unlock(client);

  if buf != [0x4cu8, 0x43u8, 0x01u8] {
    return Err("Handshake failed".to_owned());
  }

  let stream2 = stream.clone();
  let messages = thread::spawn(move || {
    // TODO
  });

  let stream = stream2;
  let watch = thread::spawn(move || {
    // TODO
  });

  messages.join().unwrap();
  watch.join().unwrap();

  Ok(())
}

fn main() {
  tauri::Builder::default()
    .invoke_handler(tauri::generate_handler![connect])
    .run(tauri::generate_context!())
    .expect("Error while running Tauri application");
}
