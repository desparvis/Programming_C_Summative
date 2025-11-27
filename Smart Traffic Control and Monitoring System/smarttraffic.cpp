// C++ code
// Smart Traffic Control and Monitoring System


// Defining the pins for the traffic lights and buttons
#define A_RED       13
#define A_YELLOW    12
#define A_GREEN     11
#define B_RED       10
#define B_YELLOW     9
#define B_GREEN      8

// Push Buttons simulate vehicle detection
#define BTN_A        2
#define BTN_B        3

// Timing constants
const int MIN_GREEN_TIME = 4000;  // Minimum time for a green light (ms)
const int MAX_GREEN_TIME = 15000; // Maximum time for a green light (ms)
const int YELLOW_TIME    = 2000;  // Standard yellow light time (ms)
const int TIME_PER_VEHICLE = 500; // Additional green time per waiting vehicle (ms)
const int LOGGING_INTERVAL = 10000; // Log data every 10 seconds (ms)

// Enums and Structs for Dynamic Data Handling

// Signal states for the FSM
enum SignalState {
  GREEN,
  YELLOW,
  RED
};

// Structure to hold lane/intersection data
struct TrafficLane {
  int laneID;
  int redPin, yellowPin, greenPin;
  int buttonPin;
  volatile long vehicleCount; // volatile for button ISR/non-blocking read
  SignalState currentState;
  unsigned long greenDuration; // Dynamically calculated duration
  unsigned long startTime;     // Time when the current phase started
  unsigned long totalWaitTime; // Cumulative wait time (simulated)
};

// --- GLOBAL VARIABLES ---
TrafficLane lanes[2]; // Array of two TrafficLane structures (Intersection A & B)
int currentLaneIndex = 0; // Index of the lane currently in a GREEN/YELLOW phase
unsigned long nextLogTime = 0; // Timer for the logging function

// --- FUNCTION PROTOTYPES ---
void setupLane(TrafficLane *lane, int id, int rP, int yP, int gP, int bP);
void updateSignals();
void transitionState(TrafficLane *lane, SignalState newState);
void checkTrafficSensors();
void dynamicTiming(TrafficLane *lane);
void logData();
void checkSerialInterface();

// --- SETUP ---
void setup() {
  Serial.begin(9600);
  Serial.println("--- Smart Traffic System Initialized ---");
  
  // Initialize Lane A
  setupLane(&lanes[0], 0, A_RED, A_YELLOW, A_GREEN, BTN_A);
  // Initialize Lane B
  setupLane(&lanes[1], 1, B_RED, B_YELLOW, B_GREEN, BTN_B);

  // Initial State: A is GREEN, B is RED
  transitionState(&lanes[0], GREEN);
  transitionState(&lanes[1], RED);
  
  nextLogTime = millis() + LOGGING_INTERVAL;
}

// Helper to set up a lane struct and pins
void setupLane(TrafficLane *lane, int id, int rP, int yP, int gP, int bP) {
  lane->laneID = id;
  lane->redPin = rP;
  lane->yellowPin = yP;
  lane->greenPin = gP;
  lane->buttonPin = bP;
  
  // Pin setup
  pinMode(rP, OUTPUT);
  pinMode(yP, OUTPUT);
  pinMode(gP, OUTPUT);
  pinMode(bP, INPUT_PULLUP);
  
  // Initial values
  lane->vehicleCount = 0;
  lane->currentState = RED;
  lane->greenDuration = MIN_GREEN_TIME;
  lane->startTime = 0;
  lane->totalWaitTime = 0;
}

// --- MAIN LOOP (Simulated Multithreading / Concurrency) ---
void loop() {
  // 1. Signal Control (FSM) - Non-Blocking
  updateSignals(); 

  // 2. Vehicle Detection - Non-Blocking
  checkTrafficSensors(); 
  
  // 3. Logging and Monitoring - Non-Blocking Timer
  if (millis() >= nextLogTime) {
    logData();
    nextLogTime = millis() + LOGGING_INTERVAL;
  }

  // 4. System Interface (Manual Override, Stats) - Non-Blocking
  checkSerialInterface();
}

// --- IMPLEMENTATIONS ---

// Manages the state transition for a lane
void transitionState(TrafficLane *lane, SignalState newState) {
  // Error Handling: Ensure safe memory access
  if (lane == NULL) return; 

  // Turn off all lights first
  digitalWrite(lane->redPin, LOW);
  digitalWrite(lane->yellowPin, LOW);
  digitalWrite(lane->greenPin, LOW);

  // Set the new state
  lane->currentState = newState;
  lane->startTime = millis(); // Record start time for non-blocking wait

  switch (newState) {
    case GREEN:
      dynamicTiming(lane);
      digitalWrite(lane->greenPin, HIGH);
      Serial.print(F("Lane ")); Serial.print(lane->laneID); Serial.println(F(" -> GREEN"));
      // The opposing lane must be RED, this is FSM robustness
      if (lane->laneID == lanes[0].laneID) {
          digitalWrite(lanes[1].redPin, HIGH);
          lanes[1].currentState = RED;
      } else {
          digitalWrite(lanes[0].redPin, HIGH);
          lanes[0].currentState = RED;
      }
      break;
    case YELLOW:
      digitalWrite(lane->yellowPin, HIGH);
      Serial.print(F("Lane ")); Serial.print(lane->laneID); Serial.println(F(" -> YELLOW"));
      break;
    case RED:
      digitalWrite(lane->redPin, HIGH);
      Serial.print(F("Lane ")); Serial.print(lane->laneID); Serial.println(F(" -> RED"));
      // Reset vehicle count after the lane has cleared
      lane->vehicleCount = 0; 
      break;
  }
}

// FSM logic for signal cycling
void updateSignals() {
  TrafficLane *currentLane = &lanes[currentLaneIndex];
  unsigned long duration = 0;

  switch (currentLane->currentState) {
    case GREEN:
      duration = currentLane->greenDuration;
      if (millis() - currentLane->startTime >= duration) {
        transitionState(currentLane, YELLOW);
      }
      break;

    case YELLOW:
      duration = YELLOW_TIME;
      if (millis() - currentLane->startTime >= duration) {
        // Transition to RED, then switch to the next lane's GREEN
        transitionState(currentLane, RED); 
        
        // Move to the next lane (switching control)
        currentLaneIndex = 1 - currentLaneIndex; 
        TrafficLane *nextLane = &lanes[currentLaneIndex];
        
        // Ensure the next lane's RED phase is active before switching
        transitionState(nextLane, GREEN);
      }
      break;

    case RED:
      // RED phase is controlled by the GREEN/YELLOW time of the *other* lane.
      // No transition needed here as the next lane's GREEN phase handles the switch.
      break;
  }
}

// Detects vehicle presence (using buttons)
void checkTrafficSensors() {
  // Lane A detection
  if (digitalRead(BTN_A) == LOW) { // Button press (LOW due to INPUT_PULLUP)
    lanes[0].vehicleCount++;
    // Simple debouncing: wait until button is released
    while(digitalRead(BTN_A) == LOW); 
  }

  // Lane B detection
  if (digitalRead(BTN_B) == LOW) {
    lanes[1].vehicleCount++;
    while(digitalRead(BTN_B) == LOW);
  }
}

// Adjust signal timing dynamically based on traffic count
void dynamicTiming(TrafficLane *lane) {
  unsigned long dynamicTime = lane->vehicleCount * TIME_PER_VEHICLE;
  lane->greenDuration = MIN_GREEN_TIME + dynamicTime;
  
  // Cap the maximum time (Robustness)
  if (lane->greenDuration > MAX_GREEN_TIME) {
    lane->greenDuration = MAX_GREEN_TIME;
  }
  
  Serial.print(F("Lane ")); Serial.print(lane->laneID); 
  Serial.print(F(" count: ")); Serial.print(lane->vehicleCount);
  Serial.print(F(" -> Duration: ")); Serial.print(lane->greenDuration / 1000.0); Serial.println(F("s"));
}

// Logs data to the Serial port which is the same as file logging
void logData() {
  Serial.println(F("\n--- LOG ENTRY ---"));
  
  // Header (for easy parsing/redirection)
  Serial.println(F("Timestamp(ms),LaneID,State,VehicleCount,CurrentGreenDuration(ms),WaitTime(ms)"));

  for (int i = 0; i < 2; i++) {
    TrafficLane *lane = &lanes[i];
     
    // If the lane is RED, its vehicles are waiting.
    unsigned long currentWait = 0;
    if (lane->currentState == RED) {
        currentWait = millis() - lane->startTime;
        lane->totalWaitTime += currentWait;
    }
    
    // Log Traffic Data in CSV format
    Serial.print(millis()); Serial.print(F(","));
    Serial.print(lane->laneID); Serial.print(F(","));
    
    // print State
    if (lane->currentState == GREEN) Serial.print(F("GREEN"));
    else if (lane->currentState == YELLOW) Serial.print(F("YELLOW"));
    else Serial.print(F("RED"));
    Serial.print(F(","));

    Serial.print(lane->vehicleCount); Serial.print(F(","));
    Serial.print(lane->greenDuration); Serial.print(F(","));
    Serial.println(lane->totalWaitTime);
  }
  Serial.println(F("-----------------"));
}

// Serial interface for status and manual override
void checkSerialInterface() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    Serial.print(F("Command received: ")); Serial.println(command);
    
    switch (command) {
      case 's': // Display Status
        Serial.println(F("\n--- SYSTEM STATUS ---"));
        Serial.print(F("Lane A Count: ")); Serial.println(lanes[0].vehicleCount);
        Serial.print(F("Lane B Count: ")); Serial.println(lanes[1].vehicleCount);
        
        break;

    }
  }
}