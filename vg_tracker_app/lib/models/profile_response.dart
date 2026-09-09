class ProfileResponse {
  final int id;
  final String username;
  final String email;

  ProfileResponse({
    required this.id,
    required this.username,
    required this.email,
  });

  factory ProfileResponse.fromJson(Map<String, dynamic> json) {
    return ProfileResponse(
      id: json['id'] as int,
      username: json['username'] as String,
      email: json['email'] as String,
    );
  }
}
