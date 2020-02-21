/**
 * This file is a barebones skeleton of how to setup an arm for use.
 * It demonstrates gravity compensation behavior by commanding torques
 * equal to the force from gravity on the links and joints of an arm.
 * Note that this only approximately balances out gravity, as imperfections in
 * the torque sensing and modeled system can lead to "drift".  Also, the
 * particular choice of PID control gains can affect the performance of this
 * demo.
 */

// #include "lookup.hpp"
// #include "group.hpp"
#include "group_command.hpp"
#include "group_feedback.hpp"
#include "robot_model.hpp"
#include "arm/arm.hpp"
#include "util/mobile_io.hpp"
#include <chrono>

using namespace hebi;
using namespace experimental; 


// double currentTime(std::chrono::steady_clock::time_point& start) {
//   return (std::chrono::duration<double>(std::chrono::steady_clock::now() - start)).count();
// }

int main(int argc, char* argv[])
{
  //////////////////////////
  ///// Arm Setup //////////
  //////////////////////////

  arm::Arm::Params params;

  // Setup Module Family and Module Names
  params.families_ = {"Arm"};
  params.names_ = {"J1_base", "J2_shoulder", "J3_elbow", "J4_wrist1", "J5_wrist2", "J6_wrist3"};

  // Read HRDF file to seutp a RobotModel object for the 6-DoF Arm
  // Make sure you are running this from the correct directory!
  params.hrdf_file_ = "kits/hrdf/6-dof_arm.hrdf";

  // Create the Arm Object
  auto arm = arm::Arm::create(params);

  //////////////////////////
  //// MobileIO Setup //////
  //////////////////////////

  // Create the MobileIO object
  std::unique_ptr<MobileIO> mobile = MobileIO::create("HEBI", "mobileIO");

  std::string instructions;
  instructions = ("B1 - Waypoint 1\nB2 - Waypoint 2\n"
                  "B3 - Waypoint 3\n"
                  "B6 - Grav comp mode\nB8 - Quit\n");
  // Clear any garbage on screen
  mobile -> clearText(); 

  // Display instructions on screen
  mobile -> sendText(instructions); 

  // Setup instructions
  auto last_state = mobile->getState();

  /////////////////////////////
  // Control Variables Setup //
  /////////////////////////////

  // Single Waypoint Vectors
  auto num_joints = arm -> robotModel().getDoFCount();
  Eigen::VectorXd positions(num_joints);
  double single_time;
  single_time = 3;


  //////////////////////////
  //// Main Control Loop ///
  //////////////////////////

  while(arm->update())
  {
    // Get latest mobile_state
    auto state = mobile->getState();
    MobileIODiff diff(last_state, state);

    /////////////////
    // Button Presses
    /////////////////

    // Buttton B1 - Home Position
    if (diff.get(1) == MobileIODiff::ButtonState::ToOn)
    {
      positions << 0, 0, 0, 0, 0, 0;
      arm -> setGoal(arm::Goal::createFromPosition(single_time, positions));
    }

    // Button B2 - Waypoint 1
    if (diff.get(2) == MobileIODiff::ButtonState::ToOn)
    {
      positions << M_PI/4, M_PI/3, 2*M_PI/3, M_PI/3, M_PI/4, 0;
      arm -> setGoal(arm::Goal::createFromPosition(single_time, positions));
    }

    // Button B3 - Waypoint 2
    if (diff.get(3) == MobileIODiff::ButtonState::ToOn)
    {
      positions << -M_PI/4, M_PI/3, 2*M_PI/3, M_PI/3, 3*M_PI/4, 0;
      arm -> setGoal(arm::Goal::createFromPosition(single_time, positions));

    }

    // Button B6 - Grav Comp Mode
    if (diff.get(6) == MobileIODiff::ButtonState::ToOn)
    {
      // cancel any goal that is set, returning arm into gravComp mode
      arm -> cancelGoal();
    }

    // Button B8 - End Demo
    if (diff.get(8) == experimental::MobileIODiff::ButtonState::ToOn)
    {
      // Clear MobileIO text
      mobile -> clearText();
      return 1;
    }

    /////////////////
    // Update & send
    /////////////////

    // Update to the new last_state for mobile device
    last_state = state;

    // Send latest commands to the arm
    arm->send();
  }

  // Clear MobileIO text
  mobile -> clearText();

  return 0;
}



