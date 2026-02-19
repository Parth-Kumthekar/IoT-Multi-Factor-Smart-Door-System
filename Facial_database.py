import cv2
import dlib
import sqlite3
import numpy as np
import pickle

# Initialize dlib models
detector = dlib.get_frontal_face_detector()
sp = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")
facerec = dlib.face_recognition_model_v1(
    "dlib_face_recognition_resnet_model_v1.dat"
)

# Create database
conn = sqlite3.connect("faces.db")
cursor = conn.cursor()

cursor.execute("""
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,
    embedding BLOB
)
""")

name = input("Enter user name: ")

cap = cv2.VideoCapture(0)
print("Look at camera...")

ret, frame = cap.read()
cap.release()

if not ret:
    print("Camera error")
    exit()

rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
faces = detector(rgb)

if len(faces) == 0:
    print("No face detected")
    exit()

shape = sp(rgb, faces[0])
embedding = np.array(facerec.compute_face_descriptor(rgb, shape))

blob = pickle.dumps(embedding)

cursor.execute("INSERT INTO users (name, embedding) VALUES (?, ?)", (name, blob))
conn.commit()

print("User enrolled successfully!")

conn.close()
