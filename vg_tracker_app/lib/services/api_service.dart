import 'dart:convert';

import 'package:http/http.dart' as http;

import '../config/api_config.dart';
import 'storage_service.dart';

class ApiService {
  final StorageService _storageService = StorageService();

  Future<Map<String, String>> _getHeaders({
    bool authenticated = true,
  }) async {
    final headers = <String, String>{
      'Content-Type': 'application/json',
      'Accept': 'application/json',
    };

    if (authenticated) {
      final token = await _storageService.getToken();

      if (token != null && token.isNotEmpty) {
        headers['Authorization'] = 'Bearer $token';
      }
    }

    return headers;
  }

  Future<http.Response> get(
    String endpoint, {
    bool authenticated = true,
  }) async {
    final url = Uri.parse(
      '${ApiConfig.baseUrl}$endpoint',
    );

    final headers = await _getHeaders(
      authenticated: authenticated,
    );

    final response = await http.get(
      url,
      headers: headers,
    );

    await _handleUnauthorized(response);

    return response;
  }

  Future<http.Response> post(
    String endpoint,
    Map<String, dynamic> body, {
    bool authenticated = true,
  }) async {
    final url = Uri.parse(
      '${ApiConfig.baseUrl}$endpoint',
    );

    final headers = await _getHeaders(
      authenticated: authenticated,
    );

    final response = await http.post(
      url,
      headers: headers,
      body: jsonEncode(body),
    );

    await _handleUnauthorized(response);

    return response;
  }

  Future<http.Response> put(
    String endpoint,
    Map<String, dynamic> body, {
    bool authenticated = true,
  }) async {
    final url = Uri.parse(
      '${ApiConfig.baseUrl}$endpoint',
    );

    final headers = await _getHeaders(
      authenticated: authenticated,
    );

    final response = await http.put(
      url,
      headers: headers,
      body: jsonEncode(body),
    );

    await _handleUnauthorized(response);

    return response;
  }

  Future<http.Response> delete(
    String endpoint, {
    bool authenticated = true,
  }) async {
    final url = Uri.parse(
      '${ApiConfig.baseUrl}$endpoint',
    );

    final headers = await _getHeaders(
      authenticated: authenticated,
    );

    final response = await http.delete(
      url,
      headers: headers,
    );

    await _handleUnauthorized(response);

    return response;
  }

  Future<void> _handleUnauthorized(
    http.Response response,
  ) async {
    if (response.statusCode == 401) {
      await _storageService.clearAuth();
    }
  }
}