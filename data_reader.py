import pandas as pd
import numpy as np
from PIL import Image
import os
from sklearn.model_selection import train_test_split
from tensorflow.keras.utils import to_categorical
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Dropout
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.callbacks import EarlyStopping
import matplotlib.pyplot as plt

# CSV 파일 읽기
train_df = pd.read_csv('train/_annotations.csv')
test_df = pd.read_csv('test/_annotations.csv')

def load_image(row, folder):
    img_path = os.path.join(folder, row['filename'])
    img = Image.open(img_path).convert('L')
    roi = img.crop((row['xmin'], row['ymin'], row['xmax'], row['ymax']))
    roi = roi.resize((224, 224))
    return np.array(roi).reshape(224, 224, 1) / 255.0

# 데이터 준비
X_train = np.array([load_image(row, 'train') for _, row in train_df.iterrows()])
y_train = train_df['class'].values
X_test = np.array([load_image(row, 'test') for _, row in test_df.iterrows()])
y_test = test_df['class'].values

# 데이터 분할
X_train, X_val, y_train, y_val = train_test_split(X_train, y_train, test_size=0.2, random_state=42)

num_classes = len(np.unique(y_train))
y_train = to_categorical(y_train, num_classes)
y_val = to_categorical(y_val, num_classes)
y_test = to_categorical(y_test, num_classes)

print("학습 데이터 형태:", X_train.shape)
print("검증 데이터 형태:", X_val.shape)
print("테스트 데이터 형태:", X_test.shape)
print("클래스 수:", num_classes)

# 모델 정의
model = Sequential([
    Conv2D(32, (3, 3), activation='relu', input_shape=(224, 224, 1)),
    MaxPooling2D(2, 2),
    Conv2D(64, (3, 3), activation='relu'),
    MaxPooling2D(2, 2),
    Conv2D(64, (3, 3), activation='relu'),
    MaxPooling2D(2, 2),
    Flatten(),
    Dense(128, activation='relu'),
    Dropout(0.5),
    Dense(num_classes, activation='softmax')
])

# 모델 컴파일
model.compile(optimizer=Adam(learning_rate=0.001),
              loss='categorical_crossentropy',
              metrics=['accuracy'])

# 콜백 함수
early_stopping = EarlyStopping(patience=10, restore_best_weights=True)

# 모델 학습
history = model.fit(
    X_train, y_train,
    batch_size=32,
    epochs=100,
    validation_data=(X_val, y_val),
    callbacks=[early_stopping]
)

# 모델 평가
test_loss, test_accuracy = model.evaluate(X_test, y_test)
print(f"Test accuracy: {test_accuracy:.4f}")

# 모델 저장
model.save('bottleCap.h5')

# 학습 곡선
plt.figure(figsize=(12, 4))

plt.subplot(1, 2, 1)
plt.plot(history.history['accuracy'], label='Train Accuracy')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
plt.title('Model Accuracy')
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.legend()

plt.subplot(1, 2, 2)
plt.plot(history.history['loss'], label='Train Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.title('Model Loss')
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.legend()

plt.tight_layout()
plt.show()